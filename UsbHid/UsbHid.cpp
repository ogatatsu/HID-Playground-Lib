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
#include "FreeRTOS.h"
#include "HidReportDescriptor.h"
#include "semphr.h"

#define REPORT_READY_TIMEOUT_MS 1000

namespace
{
  SemaphoreHandle_t report_ready_sem;
  StaticSemaphore_t report_ready_sem_buf;

  void init_report_ready_sem()
  {
    report_ready_sem = xSemaphoreCreateBinaryStatic(&report_ready_sem_buf);
    xSemaphoreGive(report_ready_sem);
  }

  bool wait_report_ready()
  {
    if (xSemaphoreTake(report_ready_sem, pdMS_TO_TICKS(REPORT_READY_TIMEOUT_MS)) == pdTRUE)
    {
      return true;
    }
    return false;
  }

  bool notify_report_complete()
  {
    return xSemaphoreGive(report_ready_sem) == pdTRUE;
  }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint16_t len)
{
  notify_report_complete();
}

namespace hidpg
{
  namespace Internal
  {

    Adafruit_USBD_HID UsbHidClass::_usb_hid;
    UsbHidReporter UsbHidClass::_reporter;

    UsbHidReporter::UsbHidReporter() : _usb_hid(nullptr), _kbd_led_cb(nullptr)
    {
    }

    void UsbHidReporter::setUsbHid(Adafruit_USBD_HID *usb_hid)
    {
      _usb_hid = usb_hid;
    }

    bool UsbHidReporter::keyboardReport(uint8_t modifiers, uint8_t key_codes[6])
    {
      if (tud_ready() == false)
      {
        return false;
      }

      hid_keyboard_report_t report = {
          .modifier = modifiers,
          .reserved = 0,
          .keycode = {
              key_codes[0],
              key_codes[1],
              key_codes[2],
              key_codes[3],
              key_codes[4],
              key_codes[5],
          },
      };

      wait_report_ready();
      return _usb_hid->sendReport(REPORT_ID_KEYBOARD, &report, sizeof(hid_keyboard_report_t));
    }

    bool UsbHidReporter::consumerReport(uint16_t usage_code)
    {
      if (tud_ready() == false)
      {
        return false;
      }

      wait_report_ready();
      return _usb_hid->sendReport(REPORT_ID_CONSUMER_CONTROL, &usage_code, sizeof(usage_code));
    }

    bool UsbHidReporter::mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
    {
      if (tud_ready() == false)
      {
        return false;
      }

      hid_mouse_report_ex_t report = {
          buttons = buttons,
          .x = x,
          .y = y,
          .wheel = wheel,
          .pan = horiz,
      };

      wait_report_ready();
      return _usb_hid->sendReport(REPORT_ID_MOUSE, &report, sizeof(hid_mouse_report_ex_t));
    }

    bool UsbHidReporter::radialControllerReport(bool button, int16_t dial)
    {
      if (tud_ready() == false)
      {
        return false;
      }

      hid_radial_controller_report_t report = {
          .button = button,
          .dial = dial,
      };

      wait_report_ready();
      return _usb_hid->sendReport(REPORT_ID_RADIAL_CONTROLLER, &report, sizeof(hid_radial_controller_report_t));
    }

    bool UsbHidReporter::systemControlReport(uint8_t usage_code)
    {
      if (tud_ready() == false)
      {
        return false;
      }

      wait_report_ready();
      return _usb_hid->sendReport(REPORT_ID_SYSTEM_CONTROL, &usage_code, sizeof(usage_code));
    }

    bool UsbHidReporter::waitReady()
    {
      if (tud_ready() == false)
      {
        return false;
      }

      if (wait_report_ready() == false)
      {
        return false;
      }

      notify_report_complete();
      return true;
    }

    void UsbHidReporter::setKeyboardLedCallback(kbd_led_cb_t cb)
    {
      _kbd_led_cb = cb;
    }

    bool UsbHidClass::begin()
    {
      init_report_ready_sem();

      _usb_hid.setPollInterval(1);
      _usb_hid.setReportDescriptor(hid_report_descriptor, sizeof(hid_report_descriptor));
      _usb_hid.setReportCallback(NULL, UsbHidClass::hid_report_callback);
      if (_usb_hid.begin() == false)
      {
        return false;
      }
      _reporter.setUsbHid(&_usb_hid);

      return true;
    }

    void UsbHidClass::hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
    {
      if (!(report_id == REPORT_ID_KEYBOARD && report_type == HID_REPORT_TYPE_OUTPUT))
      {
        return;
      }

      if (_reporter._kbd_led_cb != nullptr)
      {
        _reporter._kbd_led_cb(buffer[0]);
      }
    }

    HidReporter *UsbHidClass::getHidReporter()
    {
      return &_reporter;
    }

  } // namespace Internal

  Internal::UsbHidClass UsbHid;

} // namespace hidpg
