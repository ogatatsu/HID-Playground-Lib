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
#include "clients/BLEClientDis.h"
#include "clients/BLEClientTrackPointKeyboard2Hid.h"

namespace hidpg
{

  class BLECentralProfileTrackpointKeyboard2 : public BLECentralProfile
  {
  public:
    BLEClientDis Dis;
    BLEClientTrackPointKeyboard2Hid Hid;

    bool begin() override;
    bool needsActiveScan() override { return true; }
    bool canConnect(ble_gap_evt_adv_report_t *report) override;
    uint16_t connHandle() override;
    bool discover(uint16_t conn_handle) override;
    bool discovered() override;
    bool enable() override;
  };

} // namespace hidpg
