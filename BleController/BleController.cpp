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

#include "BleController.h"

namespace hidpg
{

  void BleControllerClass::begin()
  {
    Bluefruit.configPrphConn(BLE_GATT_ATT_MTU_DEFAULT,
                             BLE_GAP_EVENT_LENGTH_DEFAULT,
                             BLE_HVN_TX_QUEUE_SIZE,
                             BLE_GATTC_WRITE_CMD_TX_QUEUE_SIZE_DEFAULT);

    Bluefruit.begin(1, sizeof((uint8_t[][6])BLE_SLAVE_ADDR_LIST) / 6);
    Bluefruit.setTxPower(BLE_TX_POWER);
    Bluefruit.setName(BLE_DEVICE_NAME);
    Bluefruit.autoConnLed(false);

    Periph.begin();
    Central.begin();
  }

  BleControllerClass BleController;

} // namespace hidpg
