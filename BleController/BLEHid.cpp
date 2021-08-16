/**************************************************************************/
/*!
    @file     BLEHidAdafruit.cpp
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include "BLEHid.h"
#include "HidReportDescriptor.h"

namespace hidpg::Internal
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
      svc._kbd_led_cb(data[0]);

    if (svc._kbd_led_hdl_cb != nullptr)
      svc._kbd_led_hdl_cb(conn_hdl, data[0]);
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
    hid_keyboard_report_t report;
    report.modifier = modifiers;
    memcpy(report.keycode, key_codes, 6);

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
      hid_mouse_report_t report;
      report.buttons = buttons;
      report.x = constrain(x, -127, 127);
      report.y = constrain(y, -127, 127);
      report.wheel = wheel;
      report.pan = horiz;
      return bootMouseReport(conn_hdl, &report, sizeof(hid_mouse_report_t));
    }
    else
    {
      hid_mouse_report_ex_t report;
      report.buttons = buttons;
      report.x = x;
      report.y = y;
      report.wheel = wheel;
      report.pan = horiz;
      return inputReport(conn_hdl, REPORT_ID_MOUSE, &report, sizeof(hid_mouse_report_ex_t));
    }
  }

  bool BLEHid::mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    return mouseReport(BLE_CONN_HANDLE_INVALID, buttons, x, y, wheel, horiz);
  }

  bool BLEHid::radialControllerReport(uint16_t conn_hdl, bool button, int16_t dial)
  {
    hid_radial_controller_report_t report;
    report.button = button;
    report.dial = dial;

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
      if (conn->getHvnPacket() == false)
        return false;
      conn->releaseHvnPacket();
    }
    return true;
  }

  bool BLEHid::waitReady()
  {
    uint8_t conn_hdl = Bluefruit.connHandle();
    return waitReady(conn_hdl);
  }

} // namespace hidpg::Internal
