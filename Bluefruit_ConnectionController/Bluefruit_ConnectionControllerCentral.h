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

#include "BLECentralProfile.h"
#include "BlinkLed.h"
#include "centrals/BLECentralProfileBTTB179.h"
#include "centrals/BLECentralProfileKoneProAir.h"
#include "centrals/BLECentralProfileLift.h"
#include "centrals/BLECentralProfileRelacon.h"
#include "centrals/BLECentralProfileTrackPointKeyboard2.h"
#include "centrals/BLECentralProfileUart.h"

namespace hidpg::Internal
{

  class Bluefruit_ConnectionControllerCentral
  {
    friend class Bluefruit_ConnectionControllerClass;

  public:
    using disconnectCallbackCallback_t = void (*)(uint16_t conn_handle, uint8_t reason);

    static void setProfile(BLECentralProfile *profile_list[], uint8_t profile_list_len)
    {
      _profile_list = profile_list;
      _profile_list_len = profile_list_len;
      _unconnected_count = _profile_list_len;

      for (size_t i = 0; i < profile_list_len; i++)
      {
        if (profile_list[i]->needsActiveScan())
        {
          Bluefruit.Scanner.useActiveScan(true);
          return;
        }
      }
    }

    static void setProfile(BLECentralProfile *profile)
    {
      static BLECentralProfile *profile_list[] = {profile};
      setProfile(profile_list, 1);
    }

    static void setProfile(BLECentralProfile *profile1, BLECentralProfile *profile2)
    {
      static BLECentralProfile *profile_list[] = {profile1, profile2};
      setProfile(profile_list, 2);
    }

    static void setProfile(BLECentralProfile *profile1, BLECentralProfile *profile2, BLECentralProfile *profile3)
    {
      static BLECentralProfile *profile_list[] = {profile1, profile2, profile3};
      setProfile(profile_list, 3);
    }

    static void setScanLed(BlinkLed *scan_led);
    static void start();
    static void stop();
    static bool isRunning();
    static void setDisconnectCallback(disconnectCallbackCallback_t callback);

  private:
    static void begin();
    static void _start();
    static void _stop();
    static bool startScan();
    static void scan_callback(ble_gap_evt_adv_report_t *report);
    static void connect_callback(uint16_t conn_handle);
    static void connection_secured_callback(uint16_t conn_handle);
    static void disconnect_callback(uint16_t conn_handle, uint8_t reason);

    static BLECentralProfile **_profile_list;
    static uint8_t _profile_list_len;
    static uint8_t _unconnected_count;
    static bool _is_running;
    static disconnectCallbackCallback_t _disconnect_callback;

    static BlinkLed *_scan_led;
  };

} // namespace hidpg::Internal
