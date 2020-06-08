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

#include "BleHidReporter.h"

namespace hidpg
{

  BleHidReporter::BleHidReporter(BLEHidAdafruit &ble_hid) : _ble_hid(ble_hid)
  {
  }

  void BleHidReporter::keyboardReport(uint8_t modifier, uint8_t key_codes[6])
  {
    _ble_hid.keyboardReport(modifier, key_codes);
  }

  void BleHidReporter::consumerReport(uint16_t usage_code)
  {
    _ble_hid.consumerReport(usage_code);
  }

  void BleHidReporter::mouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t horiz)
  {
    _ble_hid.mouseReport(buttons, x, y, wheel, horiz);
  }

} // namespace hidpg
