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

#include "HidCore.h"
#include "ArduinoMacro.h"
#include "HidEngine_config.h"
#include "Set.h"
#include "task.h"
#include <string.h>

#define KEY_REPORT_MIN_INTERVAL_TICKS (pdMS_TO_TICKS(HID_ENGINE_KEY_REPORT_MIN_INTERVAL_MS))

namespace hidpg
{
  namespace Internal
  {

    HidReporter *HidCore::_hid_reporter = nullptr;
    uint8_t HidCore::_pressed_keys[7] = {};
    uint8_t HidCore::_prev_sent_keys[6] = {};
    uint8_t HidCore::_key_counters[256] = {};
    uint8_t HidCore::_prev_sent_modifiers = 0;
    uint8_t HidCore::_modifier_counters[8] = {};
    uint8_t HidCore::_prev_sent_mouse_buttons = 0;
    uint8_t HidCore::_mouse_button_counters[5] = {};
    bool HidCore::_prev_sent_radial_button = false;
    uint8_t HidCore::_radial_button_counter = 0;
    portTickType HidCore::_last_send_ticks = 0;

    void HidCore::setReporter(HidReporter *hid_reporter)
    {
      _hid_reporter = hid_reporter;
    }

    void HidCore::waitReady()
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->waitReady();
      }
    }

    void HidCore::setKey(CharacterKey character_key)
    {
      uint8_t code = static_cast<uint8_t>(character_key);

      _key_counters[code]++;

      // すでに入ってるなら追加しない
      for (int i = 0; i < 6; i++)
      {
        if (_pressed_keys[i] == code)
        {
          return;
        }
      }
      // 開いているスペースを探して追加
      for (int i = 0; i < 6; i++)
      {
        if (_pressed_keys[i] == 0)
        {
          _pressed_keys[i] = code;
          return;
        }
      }
      // 満杯ならずらして末尾に追加
      memmove(_pressed_keys, _pressed_keys + 1, 5);
      _pressed_keys[5] = code;
    }

    void HidCore::unsetKey(CharacterKey character_key)
    {
      uint8_t code = static_cast<uint8_t>(character_key);

      _key_counters[code]--;

      if (_key_counters[code] == 0)
      {
        int i = 0;
        // 探して削除
        for (; i < 6; i++)
        {
          if (_pressed_keys[i] == code)
          {
            _pressed_keys[i] = 0;
            break;
          }
        }
        // 削除したスペースを埋めるためにずらしていく
        // _pressed_keys[6]は常に0が入っているので末尾には0が補充される
        for (; i < 6; i++)
        {
          _pressed_keys[i] = _pressed_keys[i + 1];
        }
      }
    }

    template <typename T, size_t SIZE>
    static void countUp(T (&counters)[SIZE], uint8_t bitmap)
    {
      for (size_t i = 0; i < SIZE; i++)
      {
        if (bitRead(bitmap, i))
        {
          counters[i]++;
        }
      }
    }

    template <typename T, size_t SIZE>
    static void countDown(T (&counters)[SIZE], uint8_t bitmap)
    {
      for (size_t i = 0; i < SIZE; i++)
      {
        if (bitRead(bitmap, i))
        {
          counters[i]--;
        }
      }
    }

    void HidCore::setModifiers(Modifiers modifiers)
    {
      countUp(_modifier_counters, static_cast<uint8_t>(modifiers));
    }

    void HidCore::unsetModifiers(Modifiers modifiers)
    {
      countDown(_modifier_counters, static_cast<uint8_t>(modifiers));
    }

    bool HidCore::isModifiersSet()
    {
      for (int i = 0; i < 8; i++)
      {
        if (_modifier_counters[i] != 0)
        {
          return true;
        }
      }
      return false;
    }

    void HidCore::sendKeyReport()
    {
      // 前回送ったreportと比較して変更があるか
      bool is_changed = false;
      // 新しくkeyもしくはmodifierが追加されたか、減った場合はfalseのまま
      bool is_key_adding = false;
      bool is_modifier_adding = false;

      // 前回の6keyとの比較して変更があるか計算する
      if (memcmp(_prev_sent_keys, _pressed_keys, sizeof(_prev_sent_keys)) != 0)
      {
        is_changed = true;

        Set prev, current;
        prev.addAll(_prev_sent_keys, 6);
        current.addAll(_pressed_keys, 6);
        is_key_adding = ((current - prev).count() != 0);
      }

      // 現在押されているmodifierを追加
      uint8_t modifiers = 0;
      for (int i = 0; i < 8; i++)
      {
        if (_modifier_counters[i] > 0)
        {
          modifiers |= bit(i);
        }
      }

      // 前回のmodifierとの比較
      if (modifiers != _prev_sent_modifiers)
      {
        is_changed = true;
        // modifierが新しく追加されたかどうかを計算
        is_modifier_adding = ((modifiers & ~_prev_sent_modifiers) != 0);
      }

      if (is_key_adding && is_modifier_adding)
      {
        // keyとmodifierが同時に追加された場合はmodifierキーを送ってからkeyを送る
        // 全く同じタイミングで送ると一部の環境で意図しない動きになる（windowsキーを使ったショートカットなど）
        if (_hid_reporter != nullptr)
        {
          vTaskDelayUntil(&_last_send_ticks, KEY_REPORT_MIN_INTERVAL_TICKS);
          _hid_reporter->keyboardReport(modifiers, _prev_sent_keys);
          vTaskDelay(KEY_REPORT_MIN_INTERVAL_TICKS);
          _hid_reporter->keyboardReport(modifiers, _pressed_keys);
          _last_send_ticks = xTaskGetTickCount();
        }
      }
      else if (is_changed)
      {
        if (_hid_reporter != nullptr)
        {
          vTaskDelayUntil(&_last_send_ticks, KEY_REPORT_MIN_INTERVAL_TICKS);
          _hid_reporter->keyboardReport(modifiers, _pressed_keys);
          _last_send_ticks = xTaskGetTickCount();
        }
      }

      // 次回用に保存
      if (is_changed)
      {
        memcpy(_prev_sent_keys, _pressed_keys, sizeof(_prev_sent_keys));
        _prev_sent_modifiers = modifiers;
      }
    }

    void HidCore::consumerControlPress(ConsumerControlCode usage_code)
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->consumerReport(static_cast<uint16_t>(usage_code));
      }
    }

    void HidCore::consumerControlRelease()
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->consumerReport(0);
      }
    }

    void HidCore::systemControlPress(SystemControlCode usage_code)
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->systemControlReport(static_cast<uint8_t>(usage_code));
      }
    }

    void HidCore::systemControlRelease()
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->systemControlReport(0);
      }
    }

    void HidCore::mouseMove(int16_t x, int16_t y)
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->mouseReport(_prev_sent_mouse_buttons, x, y, 0, 0);
      }
    }

    void HidCore::mouseScroll(int8_t scroll, int8_t horiz)
    {
      if (_hid_reporter != nullptr)
      {
        vTaskDelayUntil(&_last_send_ticks, KEY_REPORT_MIN_INTERVAL_TICKS);
        _hid_reporter->mouseReport(_prev_sent_mouse_buttons, 0, 0, scroll, horiz);
        _last_send_ticks = xTaskGetTickCount();
      }
    }

    void HidCore::mouseButtonsPress(MouseButtons buttons)
    {
      countUp(_mouse_button_counters, static_cast<uint8_t>(buttons));
      sendMouseButtonsReport();
    }

    void HidCore::mouseButtonsRelease(MouseButtons buttons)
    {
      countDown(_mouse_button_counters, static_cast<uint8_t>(buttons));
      sendMouseButtonsReport();
    }

    void HidCore::sendMouseButtonsReport()
    {
      uint8_t buttons = 0;

      for (int i = 0; i < 5; i++)
      {
        if (_mouse_button_counters[i] > 0)
        {
          buttons |= bit(i);
        }
      }
      if (buttons != _prev_sent_mouse_buttons)
      {
        _prev_sent_mouse_buttons = buttons;
        if (_hid_reporter != nullptr)
        {
          vTaskDelayUntil(&_last_send_ticks, KEY_REPORT_MIN_INTERVAL_TICKS);
          _hid_reporter->mouseReport(buttons, 0, 0, 0, 0);
          _last_send_ticks = xTaskGetTickCount();
        }
      }
    }

    void HidCore::radialControllerButtonPress()
    {
      _radial_button_counter++;
      sendRadialControllerButtonReport();
    }

    void HidCore::radialControllerButtonRelease()
    {
      _radial_button_counter--;
      sendRadialControllerButtonReport();
    }

    void HidCore::radialControllerDialRotate(int16_t deci_degree)
    {
      if (_hid_reporter != nullptr)
      {
        _hid_reporter->radialControllerReport(_prev_sent_radial_button, deci_degree);
      }
    }

    void HidCore::sendRadialControllerButtonReport()
    {
      bool button = (_radial_button_counter > 0) ? true : false;

      if (button != _prev_sent_radial_button)
      {
        _prev_sent_radial_button = button;
        if (_hid_reporter != nullptr)
        {
          _hid_reporter->radialControllerReport(button, 0);
        }
      }
    }

  } // namespace Internal

  Internal::HidCore Hid;

} // namespace hidpg
