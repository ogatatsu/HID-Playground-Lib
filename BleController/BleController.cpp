/*
  The MIT License (MIT)

  Copyright (c) 2019 ogatatsu.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "BleController.h"

namespace hidpg
{

#define SLOT_FILE_NAME "Slot"
#define STORE_DIR_ENAME "AddrStore"

BLEDis BleController::_bledis;
BLEBas BleController::_blebas;
BLEHidAdafruit BleController::_blehid;
BLEHidAdafruitHidReporter BleController::_hidReporter(_blehid);
BlinkLed BleController::_advLed(ADV_LED_PIN, IS_HIGH_DRIVE);
MemStore BleController::_addrStore(STORE_DIR_ENAME);
uint8_t BleController::_currentSlot;
BleController::prphCannotConnectCallback_t BleController::_cannotConnectCallback = nullptr;

// Random Static Addressを生成
static void genRandomAddr(uint8_t addr[6])
{
  while (sd_rand_application_vector_get(addr, 6) != NRF_SUCCESS)
  {
    delay(1);
  }
  // LSB format
  // Random Static Address (Upper 2 bits = 11)
  addr[5] |= 0xC0;
}

/*------------------------------------------------------------------*/
/* public
 *------------------------------------------------------------------*/
void BleController::init()
{
  Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT, BLE_GAP_EVENT_LENGTH_DEFAULT, HVN_TX_QUEUE_SIZE, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);

#ifdef CENTRAL_ENABLE
  Bluefruit.begin(1, arrcount(_slaveAddrList));
#else
  Bluefruit.begin(1, 0);
#endif

  Bluefruit.setTxPower(TX_POWER);
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.autoConnLed(false);

  Bluefruit.Periph.setConnectCallback(prph_connect_callback);
  Bluefruit.Periph.setDisconnectCallback(prph_disconnect_callback);

  // Configure and Start Device Information Service
  _bledis.setManufacturer(MANUFACTURER_NAME);
  _bledis.setModel(MODEL_NUMBER);
  _bledis.begin();

  // Start BLE Battery Service
  _blebas.begin();

  /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min
   * and max
   * connection interval to 11.25  ms and 15 ms respectively for best
   * performance.
   */
  _blehid.begin();
  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
   */
  /* Bluefruit.setConnInterval(9, 12); */

  // Storeの初期化とcurrentSlotのロード
  _addrStore.init();

  if (_addrStore.load(SLOT_FILE_NAME, &_currentSlot, sizeof(_currentSlot)) == false)
  {
    _currentSlot = 1;
  }

#ifdef CENTRAL_ENABLE
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    // Invalid all connection handle
    _slaves[i].connHandle = BLE_CONN_HANDLE_INVALID;

    // All of BLE Central Uart Serivce
    _slaves[i].bleuart.begin();
    _slaves[i].bleuart.setRxCallback(bleuart_rx_callback);
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);
  Bluefruit.Central.setConnInterval(CENTRAL_CONNECTION_INTERVAL, CENTRAL_CONNECTION_INTERVAL);

  /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Filter only accept bleuart service
   * - Don't use active scan
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(BLEUART_UUID_SERVICE);
  Bluefruit.Scanner.useActiveScan(false);
#endif
}

// 接続を開始、または接続先の変更
void BleController::startPrphConnection(uint8_t slot)
{
  if (slot == 0)
  {
    slot = _currentSlot;
  }

  // 指定されたslotですでに開始している場合は何もしない
  if (slot == _currentSlot && isPrphRunning())
  {
    return;
  }

  // 開始する前に前の接続を切断
  stopPrphConnection();

  // 電源off後、次に起動した時に同じslotで再接続するためにセーブしておく
  if (_currentSlot != slot)
  {
    _currentSlot = slot;
    _addrStore.save(SLOT_FILE_NAME, &_currentSlot, sizeof(_currentSlot));
  }

  char addrFileName[] = "000";
  addrFileName[0] += _currentSlot / 100;
  addrFileName[1] += _currentSlot % 100 / 10;
  addrFileName[2] += _currentSlot % 10;

  // slot番号に対応したアドレスをロード、無ければ生成してセーブする
  uint8_t addr[6];
  if (_addrStore.load(addrFileName, addr, sizeof(addr)) == false)
  {
    genRandomAddr(addr);
    _addrStore.save(addrFileName, addr, sizeof(addr));
  }

  // アドレスを設定
  ble_gap_addr_t tmp;
  tmp.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
  memcpy(tmp.addr, addr, 6);
  Bluefruit.setAddr(&tmp);

  // 開始
  startAdv();
}

// ペリファラル接続もしくはアドバタイズを停止する
void BleController::stopPrphConnection()
{
  Bluefruit.Advertising.restartOnDisconnect(false);
  if (Bluefruit.Advertising.isRunning())
  {
    Bluefruit.Advertising.stop();
  }

  if (Bluefruit.Periph.connected())
  {
    Bluefruit.disconnect(Bluefruit.connHandle());

    // 切断されるまで少し待つ必要がある
    while (Bluefruit.Periph.connected())
    {
      delay(1);
    }
  }
  _advLed.syncOff();
}

bool BleController::isPrphRunning()
{
  return (Bluefruit.Advertising.isRunning() || Bluefruit.Periph.connected());
}

uint8_t BleController::currentSlot()
{
  return _currentSlot;
}

void BleController::clearBonds()
{
  Bluefruit.clearBonds();
  _addrStore.clear();
}

HidReporter *BleController::getHidReporter()
{
  return &_hidReporter;
}

void BleController::setBatteryLevel(uint8_t level)
{
  _blebas.write(level);
}

void BleController::setPrphCannnotConnectCallback(prphCannotConnectCallback_t callback)
{
  _cannotConnectCallback = callback;
}

/*------------------------------------------------------------------*/
/* private
 *------------------------------------------------------------------*/
void BleController::startAdv()
{
  Bluefruit.Advertising.clearData();

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE);

  // Include BLE HID service
  Bluefruit.Advertising.addService(_blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   *
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html
   */
  Bluefruit.Advertising.setStopCallback(adv_stop_callback);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  Bluefruit.Advertising.start(60);            // 0 = Don't stop advertising after n seconds
  _advLed.blink(_currentSlot);                // advertising status led
}

// 一定時間接続できなかった場合
void BleController::adv_stop_callback()
{
  _advLed.syncOff();
  if (_cannotConnectCallback != nullptr)
  {
    _cannotConnectCallback();
  }
}

void BleController::prph_connect_callback(uint16_t connHandle)
{
  _advLed.off();
}

void BleController::prph_disconnect_callback(uint16_t connHandle, uint8_t reason)
{
  switch (reason)
  {
  case BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION:
  {
    // 自分から切断する場合はAdvertising.restartOnDisconnect(false)に設定してあるはず
    // 特に何もすることはない
    break;
  }
  default:
  {
    // 通常はAdvertising.restartOnDisconnect(true)に設定してあるはず
    // Advertisingが自動再開されるはず、ledを点灯
    _advLed.blink(_currentSlot);
    break;
  }
  }
}

/*------------------------------------------------------------------*/
/* public central
 *------------------------------------------------------------------*/
#ifdef CENTRAL_ENABLE

constexpr uint8_t BleController::_slaveAddrList[][6];
BleController::SlaveInfo BleController::_slaves[];
BlinkLed BleController::_scanLed(SCAN_LED_PIN, IS_HIGH_DRIVE);
BleController::slaveKeyCallback_t BleController::_slaveKeyCallback = nullptr;
BleController::slaveMotionCallback_t BleController::_slaveMotionCallback = nullptr;

void BleController::startCentConnection()
{
  startScan();
}

// セントラル接続もしくはスキャンニングを停止する
void BleController::stopCentConnection()
{
  Bluefruit.Scanner.restartOnDisconnect(false);
  if (Bluefruit.Scanner.isRunning())
  {
    Bluefruit.Scanner.stop();
  }

  if (Bluefruit.Central.connected())
  {
    for (size_t i = 0; i < arrcount(_slaves); i++)
    {
      if (_slaves[i].connHandle != BLE_CONN_HANDLE_INVALID)
      {
        Bluefruit.disconnect(_slaves[i].connHandle);
      }
    }
    // 切断されるまで少し待つ必要がある
    while (Bluefruit.Central.connected())
    {
      delay(1);
    }
  }
  _scanLed.syncOff();
}

bool BleController::isCentRunnning()
{
  return (Bluefruit.Scanner.isRunning() || Bluefruit.Central.connected());
}

void BleController::setSlaveKeyCallback(slaveKeyCallback_t callback)
{
  _slaveKeyCallback = callback;
}

void BleController::setSlaveMotionCallback(slaveMotionCallback_t callback)
{
  _slaveMotionCallback = callback;
}

/*------------------------------------------------------------------*/
/* private central
 *------------------------------------------------------------------*/
int BleController::countVacantConn()
{
  int sum = 0;
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    if (_slaves[i].connHandle == BLE_CONN_HANDLE_INVALID)
    {
      sum++;
    }
  }
  return sum;
}

int BleController::findConnHandle(uint16_t connHandle)
{
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    if (connHandle == _slaves[i].connHandle)
    {
      return i;
    }
  }
  return -1;
}

int BleController::findSlaveAddr(uint8_t addr[6])
{
  for (size_t i = 0; i < arrcount(_slaveAddrList); i++)
  {
    if (memcmp(addr, _slaveAddrList[i], 6) == 0)
    {
      return i;
    }
  }
  return -1;
}

bool BleController::startScan()
{
  // まだ空きがあるなら
  int cnt = countVacantConn();
  if (cnt > 0)
  {
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
    _scanLed.blink(cnt);        // scan status led
    return true;
  }
  return false;
}

void BleController::scan_callback(ble_gap_evt_adv_report_t *report)
{
  // filter slave addr list
  if (findSlaveAddr(report->peer_addr.addr) >= 0)
  {
    Bluefruit.Central.connect(report);
  }
  else
  {
    Bluefruit.Scanner.resume();
  }
}

void BleController::cent_connect_callback(uint16_t connHandle)
{
  ble_gap_addr_t addr = Bluefruit.Connection(connHandle)->getPeerAddr();
  int idx = findSlaveAddr(addr.addr);

  // Eeek: Exceeded the number of connections !!!
  if (idx < 0)
  {
    return;
  }

  if (_slaves[idx].bleuart.discover(connHandle))
  {
    _slaves[idx].connHandle = connHandle;

    // Enable TXD's notify
    _slaves[idx].bleuart.enableTXD();

    // Continue scanning for more peripherals
    if (startScan() == false)
    {
      _scanLed.off();
    };
  }
  else
  {
    /*
    Serial.print(idx);
    Serial.println(": Can not find bleuart service.");
    */
  }
}

void BleController::cent_disconnect_callback(uint16_t connHandle, uint8_t reason)
{
  int idx = findConnHandle(connHandle);

  // Non-existant connection, something went wrong, DBG !!!
  if (idx < 0)
  {
    return;
  }

  // Mark conn handle as invalid
  _slaves[idx].connHandle = BLE_CONN_HANDLE_INVALID;

  // 切断されたらキーが押しっぱなしにならないように空のデータを送る
  if (_slaveKeyCallback != nullptr)
  {
    Set ids;
    _slaveKeyCallback(ids, idx);
  }

  // 自分から切断する場合以外はScanner.restartOnDisconnect(true)に設定
  // Scanningが自動再開されるはず、ledを点灯
  if (reason != BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION)
  {
    _scanLed.blink(countVacantConn());
  }
}

void BleController::bleuart_rx_callback(BLEClientUart &uart_svc)
{
  int idx = findConnHandle(uart_svc.connHandle());

  uint8_t header = uart_svc.read();

  if (header == 0x00) // KeyEvent
  {
    int size = uart_svc.read();
    uint8_t buf[size];
    uart_svc.read(buf, size);

    Set ids;
    ids.addAll(buf, size);

    if (_slaveKeyCallback != nullptr)
    {
      _slaveKeyCallback(ids, idx);
    }
  }
  else if (header == 0x01) // MotionEvent
  {
    uint8_t id = uart_svc.read();
    int8_t deltaX = uart_svc.read();
    int8_t deltaY = uart_svc.read();

    if (_slaveMotionCallback != nullptr)
    {
      _slaveMotionCallback(deltaX, deltaY, id, idx);
    }
  }
  else // unknown
  {
    while (uart_svc.available())
    {
      uart_svc.read();
    }
  }
}

#endif

} // namespace hidpg
