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

#include "BLECentralProfileLift.h"
#include "bluefruit.h"

namespace hidpg
{
  bool BLECentralProfileLift::begin()
  {
    VERIFY(Dis.begin());
    VERIFY(Bas.begin());
    VERIFY(Hid.begin());

    return true;
  }

  bool BLECentralProfileLift::canConnect(ble_gap_evt_adv_report_t *report)
  {
    uint8_t buf[32];

    if ((discovered() == false) && (report->type.scan_response == 0) && Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, buf, sizeof(buf)))
    {
      if (memcmp(buf, "LIFT", 4) == 0)
      {
        return true;
      }
    }
    return false;
  }

  bool BLECentralProfileLift::discover(uint16_t conn_handle)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_handle);
    char buf[32];
    conn->getPeerName(buf, sizeof(buf));

    if (memcmp(buf, "LIFT", 4) != 0)
    {
      return false;
    };

    VERIFY(Dis.discover(conn_handle));
    VERIFY(Bas.discover(conn_handle));
    VERIFY(Hid.discover(conn_handle));

    return true;
  }

  uint16_t BLECentralProfileLift::connHandle()
  {
    return Hid.connHandle();
  }

  bool BLECentralProfileLift::discovered()
  {
    return Dis.discovered() && Bas.discovered() && Hid.discovered();
  }

  bool BLECentralProfileLift::enable()
  {
    VERIFY(Bas.enableNotify());
    VERIFY(Hid.enableMouse());

    return true;
  }
}
