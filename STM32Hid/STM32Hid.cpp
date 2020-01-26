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

STM32Hid_::USBCompositeHidReporter STM32Hid_::_hid_reporter;

void STM32Hid_::init()
{
  HID_Composite_Init();
}

HidReporter *STM32Hid_::getHidReporter()
{
  return &_hid_reporter;
}

void STM32Hid_::USBCompositeHidReporter::keyboardReport(uint8_t modifier, uint8_t key_codes[6])
{
  uint8_t buf[8] = {modifier, 0, key_codes[0], key_codes[1], key_codes[2], key_codes[3], key_codes[4], key_codes[5]};
  while (HID_Composite_keyboard_sendReport(buf, 8) == false)
  {
    delay(1);
  }
}

void STM32Hid_::USBCompositeHidReporter::consumerReport(uint16_t usage_code)
{
  // todo
}

void STM32Hid_::USBCompositeHidReporter::mouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t horiz)
{
  uint8_t buf[4] = {buttons, static_cast<uint8_t>(x), static_cast<uint8_t>(y), static_cast<uint8_t>(wheel)};
  while (HID_Composite_mouse_sendReport(buf, 4) == false)
  {
    delay(1);
  }
}

STM32Hid_ STM32Hid;

} // namespace hidpg
