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

#include "HidReporter.h"
#include "KeyCode.h"

namespace hidpg
{

  // HidReporterをラップしたクラス
  class HidCore
  {
  public:
    static void setReporter(HidReporter *hid_reporter);
    static void waitReady();

    // Keyboard API
    // setKeyをした後でsendKeyReportを呼び出すことでキーを送る。
    // 何回キーをsetしたかを覚えてるので複数回同じキーコードでsetKeyを呼び出したら同じ回数unsetKeyを呼び出すまではそのキーコードは入力され続ける。
    // これにより別のスイッチに同じキーコードを割り当てたとしても正しく動作する。
    static void setKey(KeyCode key_code);
    static void unsetKey(KeyCode key_code);
    static void setModifier(Modifier modifier);
    static void unsetModifier(Modifier modifier);
    static void holdOneShotModifier(Modifier modifier);
    static void releaseOneShotModifier(Modifier modifier);
    static void sendKeyReport(bool trigger_one_shot);

    // Consumer API
    static void consumerKeyPress(ConsumerControlCode usage_code);
    static void consumerKeyRelease();

    // Mouse API
    // mouseButtonPress,Releaseは複数スイッチでの同時押しに対応
    static void mouseMove(int16_t x, int16_t y);
    static void mouseScroll(int8_t scroll, int8_t horiz);
    static void mouseButtonPress(MouseButton button);
    static void mouseButtonRelease(MouseButton button);

    // Radial Controller API
    // radialControllerButtonPress,Releaseは複数スイッチでの同時押しに対応
    static void radialControllerButtonPress();
    static void radialControllerButtonRelease();
    static void radialControllerDialRotate(int16_t deci_degree);

    // System Control API
    static void systemControlKeyPress(SystemControlCode usage_code);
    static void systemControlKeyRelease();

  private:
    static void sendMouseButtonReport();
    static void sendRadialControllerButtonReport();

    static HidReporter *_hid_reporter;

    static uint8_t _pressed_keys[7];
    static uint8_t _prev_sent_keys[6];
    static uint8_t _key_counters[256];

    static uint8_t _modifier_counters[8];
    static int32_t _one_shot_modifier_counters[8];
    static int32_t _triggered_one_shot_modifier_counters[8];

    static uint8_t _prev_sent_modifier;

    static uint8_t _prev_sent_mouse_button;
    static uint8_t _mouse_button_counters[5];

    static bool _prev_sent_radial_button;
    static uint8_t _radial_button_counter;

  };

  extern HidCore Hid;

} // namespace hidpg
