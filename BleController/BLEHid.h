/**************************************************************************/
/*!
    @file     BLEHidAdafruit.h
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

#pragma once

#include "HidReporter.h"
#include "bluefruit.h"

namespace hidpg
{

  class BLEHid : public BLEHidGeneric, public HidReporter
  {
  public:
    using kbd_led_cb1_t = void (*)(uint8_t leds_bitmap);
    using kbd_led_cb2_t = void (*)(uint16_t conn_hdl, uint8_t leds_bitmap);

    BLEHid();
    err_t begin();

    bool keyboardReport(uint16_t conn_hdl, uint8_t modifier, uint8_t key_codes[6]);
    bool consumerReport(uint16_t conn_hdl, uint16_t usage_code);
    bool radialControllerReport(uint16_t conn_hdl, bool button, int16_t dial);
    bool mouseReport(uint16_t conn_hdl, uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz);
    void setKeyboardLedCallback(kbd_led_cb2_t cb);
    bool waitReady(uint16_t conn_hdl);

    bool keyboardReport(uint8_t modifier, uint8_t key_codes[6]) override;
    bool consumerReport(uint16_t usage_code) override;
    bool mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz) override;
    bool radialControllerReport(bool button, int16_t dial) override;
    void setKeyboardLedCallback(kbd_led_cb1_t cb);
    bool waitReady();

  private:
    kbd_led_cb1_t _kbd_led_cb1;
    kbd_led_cb2_t _kbd_led_cb2;

    static void keyboard_output_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);
  };

} // namespace hidpg
