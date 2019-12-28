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

#include "BLEUartLight.h"
#include "BlinkLed.h"
#include "Set.h"
#include <bluefruit.h>

namespace hidpg
{

class BleControllerSlave
{
public:
  using prphCannotConnectCallback_t = void (*)(void);
  using receiveDataCallback_t = void (*)(uint8_t *data, uint16_t len);

  static void init();
  static void startPrphConnection();
  static void stopPrphConnection();
  static bool isPrphRunning();
  static uint16_t sendData(const uint8_t *data, uint16_t len);
  static void clearBonds();
  static void setBatteryLevel(uint8_t level);
  static void setPrphCannnotConnectCallback(prphCannotConnectCallback_t callback);
  static void setReceiveDataCallback(receiveDataCallback_t callback);

private:
  static void startAdv();
  static void adv_stop_callback();
  static void connect_callback(uint16_t conn_handle);
  static void disconnect_callback(uint16_t conn_handle, uint8_t reason);
  static void bleuart_rx_callback(uint16_t conn_handle, uint8_t *data, uint16_t len);

  static BLEUartLight _ble_uart;
  static BLEBas _ble_bas;
  static BlinkLed _adv_led;
  static prphCannotConnectCallback_t _cannot_connect_cb;
  static receiveDataCallback_t _receive_data_cb;
};

} // namespace hidpg
