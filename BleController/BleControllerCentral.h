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

#pragma once

#include "BLEClientUartLight.h"
#include "BleController_config.h"
#include "BlinkLed.h"
#include <bluefruit.h>

namespace hidpg::Internal
{

  // clang-format off
#ifndef BLE_SLAVE_ADDR_LIST
#define BLE_SLAVE_ADDR_LIST {}
#endif
  // clang-format on

#define NUM_OF_SLAVES (sizeof((uint8_t[][6])BLE_SLAVE_ADDR_LIST) / 6)

  class BleControllerCentral
  {
    friend class BleControllerClass;

  public:
    using receiveDataCallback_t = void (*)(uint8_t idx, uint8_t *data, uint16_t len);
    using disconnectCallback_t = void (*)(uint8_t idx, uint8_t reason);

    static void startConnection();
    static void stopConnection();
    static bool isRunnning();
    static uint16_t sendData(uint8_t idx, const uint8_t *data, uint16_t len);
    static void setReceiveDataCallback(receiveDataCallback_t callback);
    static void setDisconnectCallback(disconnectCallback_t callback);

  private:
    static void begin();
    static bool startScan();
    static int countVacantConn();
    static int findConnHandle(uint16_t conn_handle);
    static int findSlaveAddr(uint8_t addr[6]);
    static void scan_callback(ble_gap_evt_adv_report_t *report);
    static void connect_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
    static void bleuart_rx_callback(uint16_t conn_handle, uint8_t *data, uint16_t len);

    struct SlaveInfo
    {
      uint16_t conn_handle;
      BLEClientUartLight ble_uart;
    };

    static uint8_t _slave_addr_list[NUM_OF_SLAVES][6];
    static SlaveInfo _slaves[NUM_OF_SLAVES];
    static BlinkLed _scan_led;
    static receiveDataCallback_t _receive_data_cb;
    static disconnectCallback_t _disconnect_cb;
  };

} // namespace hidpg::Internal
