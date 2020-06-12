/*
  The MIT License (MIT)

  Copyright (c) 2020 ogatatsu.

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

#include "UsbHid.h"

namespace hidpg
{

  // Report ID
  enum
  {
    RID_KEYBOARD = 1,
    RID_MOUSE,
    RID_CONSUMER
  };

  // HID report descriptor using TinyUSB's template
  static uint8_t const desc_hid_report[] =
  {
    TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(RID_KEYBOARD), ),
    TUD_HID_REPORT_DESC_MOUSE   ( HID_REPORT_ID(RID_MOUSE), ),
    TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(RID_CONSUMER), )
  };

  Adafruit_USBD_HID UsbHidClass::_usb_hid;
  UsbHidClass::UsbHidReporter UsbHidClass::_reporter;

  void UsbHidClass::begin()
  {
    _usb_hid.setPollInterval(2);
    _usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
    _usb_hid.begin();
  }

  HidReporter *UsbHidClass::getHidReporter()
  {
    return &_reporter;
  }

  void UsbHidClass::UsbHidReporter::keyboardReport(uint8_t modifier, uint8_t key_codes[6])
  {
    _usb_hid.keyboardReport(RID_KEYBOARD, modifier, key_codes);
  }

  void UsbHidClass::UsbHidReporter::consumerReport(uint16_t usage_code)
  {
  }

  void UsbHidClass::UsbHidReporter::mouseReport(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t horiz)
  {
    _usb_hid.mouseReport(RID_MOUSE, buttons, x, y, wheel, horiz);
  }

  UsbHidClass UsbHid;

} // namespace hidpg
