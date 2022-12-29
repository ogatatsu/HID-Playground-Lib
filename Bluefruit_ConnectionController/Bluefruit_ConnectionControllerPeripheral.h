/*
  The MIT License (MIT)

  Copyright (c) 2022 ogatatsu.

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

#include "BLEPeripheralProfile.h"
#include "BlinkLed.h"
#include "peripherals/BLEPeripheralProfileHid.h"
#include "peripherals/BLEPeripheralProfileUart.h"

namespace hidpg
{
  namespace Internal
  {

    class Bluefruit_ConnectionControllerPeripheral
    {
      friend class Bluefruit_ConnectionControllerClass;

    public:
      using cannotConnectCallback_t = void (*)(void);

      static void setProfile(BLEPeripheralProfile *profile);
      static void start();
      static void stop();
      static bool isRunning();
      static bool waitReady();
      static void setCannotConnectCallback(cannotConnectCallback_t callback);
      static void setAdvLed(BlinkLed *adv_led);

    private:
      static void begin();
      static void _start();
      static void _stop();
      static bool startAdv();
      static void makeAdvData();
      static void adv_stop_callback();
      static void connect_callback(uint16_t conn_handle);
      static void disconnect_callback(uint16_t conn_handle, uint8_t reason);

      static BLEPeripheralProfile *_profile;
      static BlinkLed *_adv_led;
      static cannotConnectCallback_t _cannot_connect_cb;
      static bool _is_running;
    };

  } // namespace Internal

} // namespace hidpg
