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

#include "BleControllerSlave.h"
#include "BleControllerSlave_config.h"

namespace hidpg
{

  BLEUartLight BleControllerSlaveClass::_ble_uart;
  BLEBas BleControllerSlaveClass::_ble_bas;
  BlinkLed BleControllerSlaveClass::_adv_led(ADV_LED_PIN, ADV_LED_ACTIVE_STATE);
  BleControllerSlaveClass::cannotConnectCallback_t BleControllerSlaveClass::_cannot_connect_cb = nullptr;
  BleControllerSlaveClass::receiveDataCallback_t BleControllerSlaveClass::_receive_data_cb = nullptr;

  //------------------------------------------------------------------+
  // public
  //------------------------------------------------------------------+
  void BleControllerSlaveClass::begin()
  {
    Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT, BLE_GAP_EVENT_LENGTH_DEFAULT, HVN_TX_QUEUE_SIZE, BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);
    Bluefruit.begin();
    Bluefruit.setTxPower(TX_POWER);
    Bluefruit.setName(DEVICE_NAME);
    Bluefruit.autoConnLed(false);

    Bluefruit.Periph.setConnectCallback(connect_callback);
    Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

#ifdef OWN_ADDR
    uint8_t addr[6] = OWN_ADDR;
    ble_gap_addr_t tmp;
    tmp.addr_type = BLE_GAP_ADDR_TYPE_RANDOM_STATIC;
    memcpy(tmp.addr, addr, 6);
    Bluefruit.setAddr(&tmp);
#endif

    // Configure and Start BLE Uart Service
    _ble_uart.begin();
    _ble_uart.setRxCallback(bleuart_rx_callback);

    // Start BLE Battery Service
    _ble_bas.begin();

    // LEDの初期化
    _adv_led.begin();
  }

  // 接続を開始
  void BleControllerSlaveClass::startConnection()
  {
    // すでに開始している場合は何もしない
    if (isRunning())
    {
      return;
    }

    startAdv();
  }

  // ペリファラル接続もしくはアドバタイズを停止する
  void BleControllerSlaveClass::stopConnection()
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

  bool BleControllerSlaveClass::isRunning()
  {
    return (Bluefruit.Advertising.isRunning() || Bluefruit.Periph.connected());
  }

  uint16_t BleControllerSlaveClass::sendData(const uint8_t *data, uint16_t len)
  {
    return _ble_uart.write(data, len);
  }

  void BleControllerSlaveClass::clearBonds()
  {
    Bluefruit.clearBonds();
  }

  void BleControllerSlaveClass::setBatteryLevel(uint8_t level)
  {
    _ble_bas.write(level);
  }

  void BleControllerSlaveClass::setCannnotConnectCallback(cannotConnectCallback_t callback)
  {
    _cannot_connect_cb = callback;
  }

  void BleControllerSlaveClass::setReceiveDataCallback(receiveDataCallback_t callback)
  {
    _receive_data_cb = callback;
  }

  //------------------------------------------------------------------+
  // private
  //------------------------------------------------------------------+
  void BleControllerSlaveClass::startAdv(void)
  {
    // Advertising packet
    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addTxPower();

    // Include bleuart 128-bit uuid
    Bluefruit.Advertising.addService(_ble_uart);

    // Secondary Scan Response packet (optional)
    // Since there is no room for 'Name' in Advertising packet
    Bluefruit.ScanResponse.addName();

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
    _adv_led.blink();                           // advertising status led
  }

  // 一定時間接続できなかった場合
  void BleControllerSlaveClass::adv_stop_callback()
  {
    _adv_led.syncOff();
    if (_cannot_connect_cb != nullptr)
    {
      _cannot_connect_cb();
    }
  }

  void BleControllerSlaveClass::connect_callback(uint16_t conn_handle)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_handle);
    conn->requestConnectionParameter(CONNECTION_INTERVAL, SLAVE_LATENCY, SUPERVISION_TIMEOUT);
    conn->requestPHY();
    _adv_led.off();
  }

  void BleControllerSlaveClass::disconnect_callback(uint16_t conn_handle, uint8_t reason)
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
      _adv_led.blink();
      break;
    }
    }
  }

  void BleControllerSlaveClass::bleuart_rx_callback(uint16_t conn_handle, uint8_t *data, uint16_t len)
  {
    if (_receive_data_cb != nullptr)
    {
      _receive_data_cb(data, len);
    }
  }

  BleControllerSlaveClass BleControllerSlave;

} // namespace hidpg
