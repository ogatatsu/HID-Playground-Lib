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

#include "BLEHid.h"
#include "BleController_config.h"
#include "BlinkLed.h"
#include "HidReporter.h"
#include "MemStore.h"
#include <bluefruit.h>

namespace hidpg::Internal
{

  class BleControllerPeripheral
  {
    friend class BleControllerClass;

  public:
    using cannotConnectCallback_t = void (*)(void);

    static void startConnection(uint8_t slot = 0);
    static void stopConnection();
    static bool isRunning();
    static uint8_t getCurrentSlot();
    static void clearBonds();
    static HidReporter *getHidReporter();
    static void setBatteryLevel(uint8_t level);
    static void setCannnotConnectCallback(cannotConnectCallback_t callback);

  private:
    static void begin();
    static void startAdv();
    static void adv_stop_callback();
    static void connect_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);

    static BLEDis _ble_dis;
    static BLEBas _ble_bas;
    static BLEHid _ble_hid;
    static BlinkLed _adv_led;
    static MemStore _addr_store;
    static uint8_t _current_slot;
    static cannotConnectCallback_t _cannot_connect_cb;
  };

} // namespace hidpg::Internal
