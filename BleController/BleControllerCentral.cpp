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

#include "BleControllerCentral.h"

namespace hidpg
{

constexpr uint8_t BleControllerCentral_::_slave_addr_list[][6];
BleControllerCentral_::SlaveInfo BleControllerCentral_::_slaves[arrcount(_slave_addr_list)];
BlinkLed BleControllerCentral_::_scan_led(SCAN_LED_PIN, SCAN_LED_ACTIVE_STATE, IS_HIGH_DRIVE);
BleControllerCentral_::receiveDataCallback_t BleControllerCentral_::_receive_data_cb = nullptr;
BleControllerCentral_::disconnectCallback_t BleControllerCentral_::_disconnect_cb = nullptr;

void BleControllerCentral_::init()
{
  for (size_t i = 0; i < arrcount(_slaves); i++)
  {
    // Invalid all connection handle
    _slaves[i].conn_handle = BLE_CONN_HANDLE_INVALID;

    // All of BLE Central Uart Serivce
    _slaves[i].ble_uart.begin();
    _slaves[i].ble_uart.setRxCallback(bleuart_rx_callback);
  }

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

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
}

void BleControllerCentral_::startConnection()
{
  startScan();
}

// セントラル接続もしくはスキャンニングを停止する
void BleControllerCentral_::stopConnection()
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

bool BleControllerCentral_::isRunnning()
{
  return (Bluefruit.Scanner.isRunning() || Bluefruit.Central.connected());
}

uint16_t BleControllerCentral_::sendData(uint8_t idx, const uint8_t *data, uint16_t len)
{
  if (_slaves[idx].conn_handle == BLE_CONN_HANDLE_INVALID)
  {
    return 0;
  }
  return _slaves[idx].ble_uart.write(data, len);
}

void BleControllerCentral_::setReceiveDataCallback(receiveDataCallback_t callback)
{
  _receive_data_cb = callback;
}

void BleControllerCentral_::setDisconnectCallback(disconnectCallback_t callback)
{
  _disconnect_cb = callback;
}

int BleControllerCentral_::countVacantConn()
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

int BleControllerCentral_::findConnHandle(uint16_t conn_handle)
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

int BleControllerCentral_::findSlaveAddr(uint8_t addr[6])
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

bool BleControllerCentral_::startScan()
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

void BleControllerCentral_::scan_callback(ble_gap_evt_adv_report_t *report)
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

void BleControllerCentral_::connect_callback(uint16_t conn_handle)
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
}

void BleControllerCentral_::disconnect_callback(uint16_t conn_handle, uint8_t reason)
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
  if (_disconnect_cb != nullptr)
  {
    _disconnect_cb(idx, reason);
  }

  // 自分から切断する場合以外はScanner.restartOnDisconnect(true)に設定
  // Scanningが自動再開されるはず、ledを点灯
  if (reason != BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION)
  {
    _scan_led.blink(countVacantConn());
  }
}

void BleControllerCentral_::bleuart_rx_callback(uint16_t conn_handle, uint8_t *data, uint16_t len)
{
  int idx = findConnHandle(conn_handle);

  if (_receive_data_cb != nullptr)
  {
    _receive_data_cb(idx, data, len);
  }
}

BleControllerCentral_ BleControllerCentral;

} // namespace hidpg
