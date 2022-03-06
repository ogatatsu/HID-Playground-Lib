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
#include "services/BLEBas.h"
#include "services/BLEDis.h"
#include "services/BLEUartNonStream.h"

namespace hidpg
{

  class BLEPeripheralProfileUart : public BLEPeripheralProfile
  {
  public:
    BLEDis Dis;
    BLEBas Bas;
    BLEUartNonStream Uart;

    bool begin() override
    {
      VERIFY_STATUS(Dis.begin());
      VERIFY_STATUS(Bas.begin());
      VERIFY_STATUS(Uart.begin());
      return true;
    };

    BLEPeripheralProfileUart(uint16_t appearance = BLE_APPEARANCE_UNKNOWN,
                             uint16_t connection_interval = 6,
                             uint16_t slave_latency = 65,
                             uint16_t supervision_timeout = 100)
        : _appearance(appearance),
          _connection_interval(connection_interval),
          _slave_latency(slave_latency),
          _supervision_timeout(supervision_timeout)
    {
    }

    uint16_t getAppearance() override { return _appearance; };
    BLEService &getService() override { return Uart; };
    uint16_t getConnectionInterval() override { return _connection_interval; };
    uint16_t getSlaveLatency() override { return _slave_latency; };
    uint16_t getSupervisionTimeout() override { return _supervision_timeout; };

  private:
    uint16_t _appearance;
    uint16_t _connection_interval;
    uint16_t _slave_latency;
    uint16_t _supervision_timeout;
  };

} // namespace hidpg
