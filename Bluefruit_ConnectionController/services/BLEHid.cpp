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

#include "BLEHid.h"
#include "HidReportDescriptor.h"

namespace hidpg
{

  BLEHid::BLEHid() : BLEHidGeneric(5, 1, 0), _kbd_led_cb(nullptr), _kbd_led_hdl_cb(nullptr)
  {
  }

  err_t BLEHid::begin()
  {
    uint16_t input_len[] = {sizeof(hid_keyboard_report_t), sizeof(uint16_t), sizeof(hid_mouse_report_ex_t), sizeof(uint16_t), sizeof(uint8_t)};
    uint16_t output_len[] = {1};

    setReportLen(input_len, output_len, NULL);
    enableKeyboard(true);
    enableMouse(true);
    setReportMap(hid_report_descriptor, sizeof(hid_report_descriptor));

    VERIFY_STATUS(BLEHidGeneric::begin());

    return ERROR_NONE;
  }

  void BLEHid::keyboard_output_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEHid &svc = (BLEHid &)chr->parentService();

    if (svc._kbd_led_cb != nullptr)
    {
      svc._kbd_led_cb(data[0]);
    }

    if (svc._kbd_led_hdl_cb != nullptr)
    {
      svc._kbd_led_hdl_cb(conn_hdl, data[0]);
    }
  }

  void BLEHid::setKeyboardLedCallback(kbd_led_cb_t cb)
  {
    _kbd_led_cb = cb;

    // Report mode
    this->setOutputReportCallback(REPORT_ID_KEYBOARD, cb ? keyboard_output_cb : NULL);
    // Boot mode
    _chr_boot_keyboard_output->setWriteCallback(cb ? keyboard_output_cb : NULL);
  }

  void BLEHid::setKeyboardLedCallback(kbd_led_cb_hdl_t cb)
  {
    _kbd_led_hdl_cb = cb;

    this->setOutputReportCallback(REPORT_ID_KEYBOARD, cb ? keyboard_output_cb : NULL);

    _chr_boot_keyboard_output->setWriteCallback(cb ? keyboard_output_cb : NULL);
  }

  bool BLEHid::keyboardReport(uint16_t conn_hdl, uint8_t modifiers, uint8_t key_codes[6])
  {
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

    if (isBootMode())
    {
      return bootKeyboardReport(conn_hdl, &report, sizeof(hid_keyboard_report_t));
    }
    else
    {
      return inputReport(conn_hdl, REPORT_ID_KEYBOARD, &report, sizeof(hid_keyboard_report_t));
    }
  }

  bool BLEHid::keyboardReport(uint8_t modifiers, uint8_t key_codes[6])
  {
    return keyboardReport(BLE_CONN_HANDLE_INVALID, modifiers, key_codes);
  }

  bool BLEHid::consumerReport(uint16_t conn_hdl, uint16_t usage_code)
  {
    return inputReport(conn_hdl, REPORT_ID_CONSUMER_CONTROL, &usage_code, sizeof(usage_code));
  }

  bool BLEHid::consumerReport(uint16_t usage_code)
  {
    return consumerReport(BLE_CONN_HANDLE_INVALID, usage_code);
  }

  bool BLEHid::mouseReport(uint16_t conn_hdl, uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    if (isBootMode())
    {
      hid_mouse_report_t report = {
          buttons = buttons,
          .x = static_cast<int8_t>(constrain(x, -127, 127)),
          .y = static_cast<int8_t>(constrain(y, -127, 127)),
          .wheel = wheel,
          .pan = horiz,
      };

      return bootMouseReport(conn_hdl, &report, sizeof(hid_mouse_report_t));
    }
    else
    {
      hid_mouse_report_ex_t report = {
          buttons = buttons,
          .x = x,
          .y = y,
          .wheel = wheel,
          .pan = horiz,
      };

      return inputReport(conn_hdl, REPORT_ID_MOUSE, &report, sizeof(hid_mouse_report_ex_t));
    }
  }

  bool BLEHid::mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    return mouseReport(BLE_CONN_HANDLE_INVALID, buttons, x, y, wheel, horiz);
  }

  bool BLEHid::radialControllerReport(uint16_t conn_hdl, bool button, int16_t dial)
  {
    hid_radial_controller_report_t report = {
        .button = button,
        .dial = dial,
    };

    return inputReport(conn_hdl, REPORT_ID_RADIAL_CONTROLLER, &report, sizeof(hid_radial_controller_report_t));
  }

  bool BLEHid::radialControllerReport(bool button, int16_t dial)
  {
    return radialControllerReport(BLE_CONN_HANDLE_INVALID, button, dial);
  }

  bool BLEHid::systemControlReport(uint16_t conn_hdl, uint8_t usage_code)
  {
    return inputReport(conn_hdl, REPORT_ID_SYSTEM_CONTROL, &usage_code, sizeof(usage_code));
  }

  bool BLEHid::systemControlReport(uint8_t usage_code)
  {
    return systemControlReport(BLE_CONN_HANDLE_INVALID, usage_code);
  }

  bool BLEHid::waitReady(uint16_t conn_hdl)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_hdl);

    if (conn != nullptr)
    {
      if (conn->getHvnPacket())
      {
        conn->releaseHvnPacket();
        return true;
      }
    }
    return false;
  }

  bool BLEHid::waitReady()
  {
    uint8_t conn_hdl = Bluefruit.connHandle();
    return waitReady(conn_hdl);
  }

} // namespace hidpg
