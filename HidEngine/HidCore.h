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

#pragma once

#include "FreeRTOS.h"
#include "HidReporter.h"
#include "KeyCode.h"

namespace hidpg
{
  namespace Internal
  {

    // HidReporterをラップしたクラス
    class HidCore
    {
    public:
      static void setReporter(HidReporter *hid_reporter);
      static void waitReady();

      // Keyboard API
      static void keyPress(CharacterKey character_key);
      static void keyRelease(CharacterKey character_key);
      static void modifiersPress(Modifiers modifiers);
      static void modifiersRelease(Modifiers modifiers);
      static bool isModifiersSet();

      // Consumer API
      static void consumerControlPress(ConsumerControlCode usage_code);
      static void consumerControlRelease();

      // Mouse API
      // mouseButtonsPress,Releaseは複数スイッチでの同時押しに対応
      static void mouseMove(int16_t x, int16_t y);
      static void mouseScroll(int8_t scroll, int8_t horiz);
      static void mouseButtonsPress(MouseButtons buttons);
      static void mouseButtonsRelease(MouseButtons buttons);

      // Radial Controller API
      // radialControllerButtonPress,Releaseは複数スイッチでの同時押しに対応
      static void radialControllerButtonPress();
      static void radialControllerButtonRelease();
      static void radialControllerDialRotate(int16_t deci_degree);

      // System Control API
      static void systemControlPress(SystemControlCode usage_code);
      static void systemControlRelease();

    private:
      static void setKey(CharacterKey character_key);
      static void unsetKey(CharacterKey character_key);
      static void setModifiers(Modifiers modifiers);
      static void unsetModifiers(Modifiers modifiers);
      static void sendKeyReport();

      static void sendMouseButtonsReport();
      static void sendRadialControllerButtonReport();

      static HidReporter *_hid_reporter;

      static uint8_t _pressed_keys[7];
      static uint8_t _prev_sent_keys[6];
      static uint8_t _key_counters[256];

      static uint8_t _prev_sent_modifiers;
      static uint8_t _modifier_counters[8];

      static uint8_t _prev_sent_mouse_buttons;
      static uint8_t _mouse_button_counters[5];

      static bool _prev_sent_radial_button;
      static uint8_t _radial_button_counter;

      static portTickType _last_send_ticks;
    };

  } // namespace Internal

  extern Internal::HidCore Hid;

} // namespace hidpg
