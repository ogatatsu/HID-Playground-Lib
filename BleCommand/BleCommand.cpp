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

#include "BleCommand.h"
#include "Arduino.h"
#include "BatteryUtil.h"
#include "BleController.h"
#include "HidCore.h"

namespace hidpg
{
/*------------------------------------------------------------------*/
/* ConnectBluetooth
 *------------------------------------------------------------------*/
ConnectBluetooth::ConnectBluetooth(uint8_t slot) : _slot(slot)
{
}

uint8_t ConnectBluetooth::onPress(uint8_t accumulation)
{
  BleController.Periph.startConnection(_slot);
  return 1;
}

/*------------------------------------------------------------------*/
/* ResetConnection
 *------------------------------------------------------------------*/
uint8_t ResetConnection::onPress(uint8_t accumulation)
{
  BleController.Periph.clearBonds();
  NVIC_SystemReset();
  return 1;
}

/*------------------------------------------------------------------*/
/* PrintBatteryLevel
 *------------------------------------------------------------------*/
static KeyCode numToKeycode(uint8_t num)
{
  if (num == 0)
    return KeyCode::_0;
  return static_cast<KeyCode>(num + 29);
}

static void tap(KeyCode key_code)
{
  Hid.setKey(key_code);
  Hid.sendKeyReport(true);
  Hid.unsetKey(key_code);
  Hid.sendKeyReport(false);
}

uint8_t PrintBatteryLevel::onPress(uint8_t accumulation)
{
  uint8_t level = BatteryUtil.readBatteryLevel();

  KeyCode level1 = numToKeycode(level % 10);
  KeyCode level2 = numToKeycode(level % 100 / 10);
  KeyCode level3 = numToKeycode(level / 100);

  tap(level3);
  tap(level2);
  tap(level1);

  return 1;
}

} // namespace hidpg
