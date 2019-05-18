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

#include "STM32Hid.h"
#include "usbd_hidpg_if.h"
#include <Arduino.h>

namespace hidpg
{

STM32Hid::USBCompositeHidReporter STM32Hid::_hidReporter;

void STM32Hid::init()
{
  HID_Composite_Init();
}

HidReporter *STM32Hid::getHidReporter()
{
  return &_hidReporter;
}

void STM32Hid::USBCompositeHidReporter::keyboardReport(uint8_t modifier, uint8_t keycode[6])
{
  uint8_t buf[8] = {modifier, 0, keycode[0], keycode[1], keycode[2], keycode[3], keycode[4], keycode[5]};
  while (HID_Composite_keyboard_sendReport(buf, 8) == false)
  {
    delay(1);
  }
}

void STM32Hid::USBCompositeHidReporter::consumerReport(uint16_t usageCode)
{
  // todo
}

void STM32Hid::USBCompositeHidReporter::mouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t horiz)
{
  uint8_t buf[4] = {buttons, static_cast<uint8_t>(x), static_cast<uint8_t>(y), static_cast<uint8_t>(wheel)};
  while (HID_Composite_mouse_sendReport(buf, 4) == false)
  {
    delay(1);
  }
}

} // namespace hidpg
