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
#include "Arduino.h"
#include "HidReportDescriptor.h"

namespace hidpg
{
  Adafruit_USBD_HID UsbHidClass::_usb_hid;
  UsbHidClass::UsbHidReporter UsbHidClass::_reporter;

  bool UsbHidClass::begin()
  {
    _usb_hid.setPollInterval(2);
    _usb_hid.enableOutEndpoint(true);
    _usb_hid.setReportDescriptor(hid_report_descriptor, sizeof(hid_report_descriptor));
    return _usb_hid.begin();
  }

  HidReporter *UsbHidClass::getHidReporter()
  {
    return &_reporter;
  }

  bool UsbHidClass::UsbHidReporter::keyboardReport(uint8_t modifier, uint8_t key_codes[6])
  {
    if (waitReady())
    {
      return _usb_hid.keyboardReport(REPORT_ID_KEYBOARD, modifier, key_codes);
    }
    return false;
  }

  bool UsbHidClass::UsbHidReporter::consumerReport(uint16_t usage_code)
  {
    if (waitReady())
    {
      return _usb_hid.sendReport(REPORT_ID_CONSUMER_CONTROL, &usage_code, sizeof(usage_code));
    }
    return false;
  }

  bool UsbHidClass::UsbHidReporter::mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    hid_mouse_report_ex_t report;
    report.buttons = buttons;
    report.x = x;
    report.y = y;
    report.wheel = wheel;
    report.pan = horiz;

    if (waitReady())
    {
      return _usb_hid.sendReport(REPORT_ID_MOUSE, &report, sizeof(hid_mouse_report_ex_t));
    }
    return false;
  }

  bool UsbHidClass::UsbHidReporter::radialControllerReport(bool button, int16_t dial)
  {
    hid_radial_controller_report_t report;
    report.button = button;
    report.dial = dial;

    if (waitReady())
    {
      return _usb_hid.sendReport(REPORT_ID_RADIAL_CONTROLLER, &report, sizeof(hid_radial_controller_report_t));
    }
    return false;
  }

  bool UsbHidClass::UsbHidReporter::systemControlReport(uint8_t usage_code)
  {
    if (waitReady())
    {
      return _usb_hid.sendReport(REPORT_ID_SYSTEM_CONTROL, &usage_code, sizeof(usage_code));
    }
    return false;
  }

  bool UsbHidClass::UsbHidReporter::waitReady()
  {
    int count = 0;
    while (_usb_hid.ready() == false)
    {
      count++;
      if (count > 10)
        return false;
      delay(1);
    }

    return true;
  }

  UsbHidClass UsbHid;

} // namespace hidpg
