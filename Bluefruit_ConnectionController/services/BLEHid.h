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

#pragma once

#include "HidReporter.h"
#include "bluefruit.h"

namespace hidpg
{

  class BLEHid : public BLEHidGeneric, public HidReporter
  {
  public:
    using kbd_led_cb_hdl_t = void (*)(uint16_t conn_hdl, uint8_t leds_bitmap);

    BLEHid();
    err_t begin();

    bool keyboardReport(uint16_t conn_hdl, uint8_t modifiers, uint8_t key_codes[6]);
    bool consumerReport(uint16_t conn_hdl, uint16_t usage_code);
    bool radialControllerReport(uint16_t conn_hdl, bool button, int16_t dial);
    bool mouseReport(uint16_t conn_hdl, uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz);
    bool systemControlReport(uint16_t conn_hdl, uint8_t usage_code);
    bool waitReady(uint16_t conn_hdl);
    void setKeyboardLedCallback(kbd_led_cb_hdl_t cb);

    bool keyboardReport(uint8_t modifiers, uint8_t key_codes[6]) override;
    bool consumerReport(uint16_t usage_code) override;
    bool mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz) override;
    bool radialControllerReport(bool button, int16_t dial) override;
    bool systemControlReport(uint8_t usage_code) override;
    bool waitReady() override;
    void setKeyboardLedCallback(kbd_led_cb_t cb) override;

  private:
    kbd_led_cb_t _kbd_led_cb;
    kbd_led_cb_hdl_t _kbd_led_hdl_cb;

    static void keyboard_output_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);
  };

} // namespace hidpg
