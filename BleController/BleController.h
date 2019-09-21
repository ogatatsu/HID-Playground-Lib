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
#include "BLEHidAdafruitHidReporter.h"
#include "BleController_config.h"
#include "BlinkLed.h"
#include "HidReporter.h"
#include "MemStore.h"
#include "Set.h"
#include <bluefruit.h>

namespace hidpg
{

#ifdef SLAVE_ADDR_LIST
#define CENTRAL_ENABLE
#endif

class BleController
{
public:
  using prphCannotConnectCallback_t = void (*)(void);

  static void init();
  static void startPrphConnection(uint8_t slot = 0);
  static void stopPrphConnection();
  static bool isPrphRunning();
  static uint8_t getCurrentSlot();
  static void clearBonds();
  static HidReporter *getHidReporter();
  static void setBatteryLevel(uint8_t level);
  static void setPrphCannnotConnectCallback(prphCannotConnectCallback_t callback);

#ifdef CENTRAL_ENABLE
  using receiveDataCallback_t = void (*)(uint8_t idx, uint8_t *data, uint16_t len);
  using centDisconnectCallback_t = void (*)(uint8_t idx, uint8_t reason);

  static void startCentConnection();
  static void stopCentConnection();
  static bool isCentRunnning();
  static void setReceiveDataCallback(receiveDataCallback_t callback);
  static void setCentDisconnectCallback(centDisconnectCallback_t callback);
#endif

private:
  static void startAdv();
  static void adv_stop_callback();
  static void prph_connect_callback(uint16_t connHandle);
  static void prph_disconnect_callback(uint16_t connHandle, uint8_t reason);

  static BLEDis _bledis;
  static BLEBas _blebas;
  static BLEHidAdafruit _blehid;
  static BLEHidAdafruitHidReporter _hidReporter;
  static BlinkLed _advLed;
  static MemStore _addrStore;
  static uint8_t _currentSlot;
  static prphCannotConnectCallback_t _cannotConnectCallback;

#ifdef CENTRAL_ENABLE
  static bool startScan();
  static int countVacantConn();
  static int findConnHandle(uint16_t connHandle);
  static int findSlaveAddr(uint8_t addr[6]);
  static void scan_callback(ble_gap_evt_adv_report_t *report);
  static void cent_connect_callback(uint16_t connHandle);
  static void cent_disconnect_callback(uint16_t connHandle, uint8_t reason);
  static void bleuart_rx_callback(uint16_t connHandle, uint8_t *data, uint16_t len);

  struct SlaveInfo
  {
    uint16_t connHandle;
    BLEClientUartLight bleuart;
  };

  static constexpr uint8_t _slaveAddrList[][6] = SLAVE_ADDR_LIST;
  static SlaveInfo _slaves[arrcount(_slaveAddrList)];
  static BlinkLed _scanLed;
  static receiveDataCallback_t _receiveDataCallback;
  static centDisconnectCallback_t _centDisconnectCallback;
#endif
};

} // namespace hidpg
