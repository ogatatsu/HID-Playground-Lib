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

#include "BLECentralProfileUart.h"
#include "bluefruit.h"

namespace hidpg
{
  BLECentralProfileUart::BLECentralProfileUart(uint8_t peer_addr[6])
  {
    memcpy(_peer_addr, peer_addr, 6);
  }

  BLECentralProfileUart::BLECentralProfileUart(ble_addr_t peer_addr) : BLECentralProfileUart(peer_addr.addr)
  {
  }

  bool BLECentralProfileUart::begin()
  {
    VERIFY(Dis.begin());
    VERIFY(Bas.begin());
    VERIFY(Uart.begin());

    return true;
  }

  bool BLECentralProfileUart::canConnect(ble_gap_evt_adv_report_t *report)
  {
    if (discovered())
    {
      return false;
    }

    if (memcmp(_peer_addr, report->peer_addr.addr, 6) != 0)
    {
      return false;
    }

    uint8_t buf[32];

    if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_128BIT_SERVICE_UUID_COMPLETE, buf, sizeof(buf)))
    {
      if (memcmp(buf, BLEUART_UUID_SERVICE, 16) == 0)
      {
        return true;
      }
    }

    return false;
  }

  bool BLECentralProfileUart::discover(uint16_t conn_handle)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_handle);

    if (memcmp(_peer_addr, conn->getPeerAddr().addr, 6) != 0)
    {
      return false;
    }

    VERIFY(Dis.discover(conn_handle));
    VERIFY(Bas.discover(conn_handle));
    VERIFY(Uart.discover(conn_handle));

    return true;
  }

  uint16_t BLECentralProfileUart::connHandle()
  {
    return Uart.connHandle();
  }

  bool BLECentralProfileUart::discovered()
  {
    return Dis.discovered() && Bas.discovered() && Uart.discovered();
  }

  bool BLECentralProfileUart::enable()
  {
    VERIFY(Uart.enableTXD());

    return true;
  }
}
