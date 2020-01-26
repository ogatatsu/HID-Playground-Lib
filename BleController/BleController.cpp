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

BLEDis BleController_::_ble_dis;
BLEBas BleController_::_ble_bas;
BLEHidAdafruit BleController_::_ble_hid;
BLEHidAdafruitHidReporter BleController_::_hid_reporter(_ble_hid);
BlinkLed BleController_::_adv_led(ADV_LED_PIN, ADV_LED_ACTIVE_STATE, IS_HIGH_DRIVE);
MemStore BleController_::_addr_store(STORE_DIR_ENAME);
uint8_t BleController_::_current_slot;
BleController_::prphCannotConnectCallback_t BleController_::_cannot_connect_cb = nullptr;

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
void BleController_::init()
{
  Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT, BLE_GAP_EVENT_LENGTH_DEFAULT, HVN_TX_QUEUE_SIZE, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);

#ifdef CENTRAL_ENABLE
  Bluefruit.begin(1, arrcount(_slave_addr_list));
#else
  Bluefruit.begin(1, 0);
#endif

  Bluefruit.setTxPower(TX_POWER);
  Bluefruit.setName(DEVICE_NAME);
  Bluefruit.autoConnLed(false);

  Bluefruit.Periph.setConnectCallback(prph_connect_callback);
  Bluefruit.Periph.setDisconnectCallback(prph_disconnect_callback);

  // Configure and Start Device Information Service
  _ble_dis.setManufacturer(MANUFACTURER_NAME);
  _ble_dis.setModel(MODEL_NUMBER);
  _ble_dis.begin();

  // Start BLE Battery Service
  _ble_bas.begin();

  /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min
   * and max
   * connection interval to 11.25  ms and 15 ms respectively for best
   * performance.
   */
  _ble_hid.begin();
  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
   */
  /* Bluefruit.setConnInterval(9, 12); */

  // storeの初期化とcurrent_slotのロード
  _addr_store.init();
  if (_addr_store.load(SLOT_FILE_NAME, &_current_slot, sizeof(_current_slot)) == false)
  {
    _current_slot = 1;
  }

#ifdef CENTRAL_ENABLE
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    // Invalid all connection handle
    _slaves[i].conn_handle = BLE_CONN_HANDLE_INVALID;

    // All of BLE Central Uart Serivce
    _slaves[i].ble_uart.begin();
    _slaves[i].ble_uart.setRxCallback(bleuart_rx_callback);
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);

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
void BleController_::startPrphConnection(uint8_t slot)
{
  if (slot == 0)
  {
    slot = _current_slot;
  }

  // 指定されたslotですでに開始している場合は何もしない
  if (slot == _current_slot && isPrphRunning())
  {
    return;
  }

  // 開始する前に前の接続を切断
  stopPrphConnection();

  // 電源off後、次に起動した時に同じslotで再接続するためにセーブしておく
  if (_current_slot != slot)
  {
    _current_slot = slot;
    _addr_store.save(SLOT_FILE_NAME, &_current_slot, sizeof(_current_slot));
  }

  char addr_filename[] = "000";
  addr_filename[0] += _current_slot / 100;
  addr_filename[1] += _current_slot % 100 / 10;
  addr_filename[2] += _current_slot % 10;

  // slot番号に対応したアドレスをロード、無ければ生成してセーブする
  uint8_t addr[6];
  if (_addr_store.load(addr_filename, addr, sizeof(addr)) == false)
  {
    genRandomAddr(addr);
    _addr_store.save(addr_filename, addr, sizeof(addr));
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
void BleController_::stopPrphConnection()
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
  _adv_led.syncOff();
}

bool BleController_::isPrphRunning()
{
  return (Bluefruit.Advertising.isRunning() || Bluefruit.Periph.connected());
}

uint8_t BleController_::getCurrentSlot()
{
  return _current_slot;
}

void BleController_::clearBonds()
{
  Bluefruit.clearBonds();
  _addr_store.clear();
}

HidReporter *BleController_::getHidReporter()
{
  return &_hid_reporter;
}

void BleController_::setBatteryLevel(uint8_t level)
{
  _ble_bas.write(level);
}

void BleController_::setPrphCannnotConnectCallback(prphCannotConnectCallback_t callback)
{
  _cannot_connect_cb = callback;
}

/*------------------------------------------------------------------*/
/* private
 *------------------------------------------------------------------*/
void BleController_::startAdv()
{
  Bluefruit.Advertising.clearData();

  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE);

  // Include BLE HID service
  Bluefruit.Advertising.addService(_ble_hid);

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
  _adv_led.blink(_current_slot);              // advertising status led
}

// 一定時間接続できなかった場合
void BleController_::adv_stop_callback()
{
  _adv_led.syncOff();
  if (_cannot_connect_cb != nullptr)
  {
    _cannot_connect_cb();
  }
}

void BleController_::prph_connect_callback(uint16_t conn_handle)
{
  BLEConnection *conn = Bluefruit.Connection(conn_handle);
  conn->requestConnectionParameter(CONNECTION_INTERVAL, SLAVE_LATENCY, SUPERVISION_TIMEOUT);
  conn->requestPHY();
  _adv_led.off();
}

void BleController_::prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
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
    _adv_led.blink(_current_slot);
    break;
  }
  }
}

/*------------------------------------------------------------------*/
/* public central
 *------------------------------------------------------------------*/
#ifdef CENTRAL_ENABLE

constexpr uint8_t BleController_::_slave_addr_list[][6];
BleController_::SlaveInfo BleController_::_slaves[];
BlinkLed BleController_::_scan_led(SCAN_LED_PIN, SCAN_LED_ACTIVE_STATE, IS_HIGH_DRIVE);
BleController_::receiveDataCallback_t BleController_::_receive_data_cb = nullptr;
BleController_::centDisconnectCallback_t BleController_::_cent_disconnect_cb = nullptr;

void BleController_::startCentConnection()
{
  startScan();
}

// セントラル接続もしくはスキャンニングを停止する
void BleController_::stopCentConnection()
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
      if (_slaves[i].conn_handle != BLE_CONN_HANDLE_INVALID)
      {
        Bluefruit.disconnect(_slaves[i].conn_handle);
      }
    }
    // 切断されるまで少し待つ必要がある
    while (Bluefruit.Central.connected())
    {
      delay(1);
    }
  }
  _scan_led.syncOff();
}

bool BleController_::isCentRunnning()
{
  return (Bluefruit.Scanner.isRunning() || Bluefruit.Central.connected());
}

uint16_t BleController_::sendData(uint8_t idx, const uint8_t *data, uint16_t len)
{
  if (_slaves[idx].conn_handle == BLE_CONN_HANDLE_INVALID)
  {
    return 0;
  }
  return _slaves[idx].ble_uart.write(data, len);
}

void BleController_::setReceiveDataCallback(receiveDataCallback_t callback)
{
  _receive_data_cb = callback;
}

void BleController_::setCentDisconnectCallback(centDisconnectCallback_t callback)
{
  _cent_disconnect_cb = callback;
}

/*------------------------------------------------------------------*/
/* private central
 *------------------------------------------------------------------*/
int BleController_::countVacantConn()
{
  int sum = 0;
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    if (_slaves[i].conn_handle == BLE_CONN_HANDLE_INVALID)
    {
      sum++;
    }
  }
  return sum;
}

int BleController_::findConnHandle(uint16_t conn_handle)
{
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    if (conn_handle == _slaves[i].conn_handle)
    {
      return i;
    }
  }
  return -1;
}

int BleController_::findSlaveAddr(uint8_t addr[6])
{
  for (size_t i = 0; i < arrcount(_slave_addr_list); i++)
  {
    if (memcmp(addr, _slave_addr_list[i], 6) == 0)
    {
      return i;
    }
  }
  return -1;
}

bool BleController_::startScan()
{
  // まだ空きがあるなら
  int cnt = countVacantConn();
  if (cnt > 0)
  {
    Bluefruit.Scanner.restartOnDisconnect(true);
    Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds
    _scan_led.blink(cnt);       // scan status led
    return true;
  }
  return false;
}

void BleController_::scan_callback(ble_gap_evt_adv_report_t *report)
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

void BleController_::cent_connect_callback(uint16_t conn_handle)
{
  ble_gap_addr_t addr = Bluefruit.Connection(conn_handle)->getPeerAddr();
  int idx = findSlaveAddr(addr.addr);

  // Eeek: Exceeded the number of connections !!!
  if (idx < 0)
  {
    return;
  }

  if (_slaves[idx].ble_uart.discover(conn_handle))
  {
    _slaves[idx].conn_handle = conn_handle;

    // Enable TXD's notify
    _slaves[idx].ble_uart.enableTXD();

    // Continue scanning for more peripherals
    if (startScan() == false)
    {
      _scan_led.off();
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

void BleController_::cent_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  int idx = findConnHandle(conn_handle);

  // Non-existant connection, something went wrong, DBG !!!
  if (idx < 0)
  {
    return;
  }

  // Mark conn handle as invalid
  _slaves[idx].conn_handle = BLE_CONN_HANDLE_INVALID;

  // invoke callback
  if (_cent_disconnect_cb != nullptr)
  {
    _cent_disconnect_cb(idx, reason);
  }

  // 自分から切断する場合以外はScanner.restartOnDisconnect(true)に設定
  // Scanningが自動再開されるはず、ledを点灯
  if (reason != BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION)
  {
    _scan_led.blink(countVacantConn());
  }
}

void BleController_::bleuart_rx_callback(uint16_t conn_handle, uint8_t *data, uint16_t len)
{
  int idx = findConnHandle(conn_handle);

  if (_receive_data_cb != nullptr)
  {
    _receive_data_cb(idx, data, len);
  }
}

#endif

BleController_ BleController;

} // namespace hidpg
