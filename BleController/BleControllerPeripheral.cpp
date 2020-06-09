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

#include "BleControllerPeripheral.h"

#define SLOT_FILE_NAME "Slot"
#define STORE_DIR_ENAME "AddrStore"

namespace hidpg
{

  BLEDis BleControllerPeripheral::_ble_dis;
  BLEBas BleControllerPeripheral::_ble_bas;
  BLEHidAdafruit BleControllerPeripheral::_ble_hid;
  BleHidReporter BleControllerPeripheral::_hid_reporter(_ble_hid);
  BlinkLed BleControllerPeripheral::_adv_led(ADV_LED_PIN, ADV_LED_ACTIVE_STATE, IS_HIGH_DRIVE);
  MemStore BleControllerPeripheral::_addr_store(STORE_DIR_ENAME);
  uint8_t BleControllerPeripheral::_current_slot;
  BleControllerPeripheral::cannotConnectCallback_t BleControllerPeripheral::_cannot_connect_cb = nullptr;

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

  void BleControllerPeripheral::init()
  {
    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

    // Configure and Start Device Information Service
    _ble_dis.setManufacturer(MANUFACTURER_NAME);
    _ble_dis.setModel(MODEL_NUMBER);
    _ble_dis.begin();

    // Start BLE Battery Service
    _ble_bas.begin();

    // Start BLE HID
    // Note: Apple requires BLE device must have min connection interval >= 20m
    // (The smaller the connection interval the faster we could send data).
    // However for HID and MIDI device, Apple could accept min connection interval
    // up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
    // connection interval to 11.25  ms and 15 ms respectively for best performance.
    _ble_hid.begin();

    // Set connection interval (min, max) to your perferred value.
    // Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
    // min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
    // Bluefruit.setConnInterval(9, 12);

    // storeの初期化とcurrent_slotのロード
    _addr_store.init();
    if (_addr_store.load(SLOT_FILE_NAME, &_current_slot, sizeof(_current_slot)) == false)
    {
      _current_slot = 1;
    }

    // LEDの初期化
    _adv_led.init();
  }

  // 接続を開始、または接続先の変更
  void BleControllerPeripheral::startConnection(uint8_t slot)
  {
    if (slot == 0)
    {
      slot = _current_slot;
    }

    // 指定されたslotですでに開始している場合は何もしない
    if (slot == _current_slot && isRunning())
    {
      return;
    }

    // 開始する前に前の接続を切断
    stopConnection();

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
  void BleControllerPeripheral::stopConnection()
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

  bool BleControllerPeripheral::isRunning()
  {
    return (Bluefruit.Advertising.isRunning() || Bluefruit.Periph.connected());
  }

  uint8_t BleControllerPeripheral::getCurrentSlot()
  {
    return _current_slot;
  }

  void BleControllerPeripheral::clearBonds()
  {
    bond_clear_prph();
    _addr_store.clear();
  }

  HidReporter *BleControllerPeripheral::getHidReporter()
  {
    return &_hid_reporter;
  }

  void BleControllerPeripheral::setBatteryLevel(uint8_t level)
  {
    _ble_bas.write(level);
  }

  void BleControllerPeripheral::setCannnotConnectCallback(cannotConnectCallback_t callback)
  {
    _cannot_connect_cb = callback;
  }

  void BleControllerPeripheral::startAdv()
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

    // Start Advertising
    // - Enable auto advertising if disconnected
    // - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
    // - Timeout for fast mode is 30 seconds
    // - Start(timeout) with timeout = 0 will advertise forever (until connected)
    // For recommended advertising interval
    // https://developer.apple.com/library/content/qa/qa1931/_index.html
    Bluefruit.Advertising.setStopCallback(adv_stop_callback);
    Bluefruit.Advertising.restartOnDisconnect(true);
    Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
    Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    Bluefruit.Advertising.start(60);            // 0 = Don't stop advertising after n seconds
    _adv_led.blink(_current_slot);              // advertising status led
  }

  // 一定時間接続できなかった場合
  void BleControllerPeripheral::adv_stop_callback()
  {
    _adv_led.syncOff();
    if (_cannot_connect_cb != nullptr)
    {
      _cannot_connect_cb();
    }
  }

  void BleControllerPeripheral::connect_callback(uint16_t conn_handle)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_handle);
    conn->requestConnectionParameter(CONNECTION_INTERVAL, SLAVE_LATENCY, SUPERVISION_TIMEOUT);
    conn->requestPHY();
    _adv_led.off();
  }

  void BleControllerPeripheral::disconnect_callback(uint16_t conn_handle, uint8_t reason)
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

} // namespace hidpg
