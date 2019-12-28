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
#include "Set.h"
#include <Arduino.h>

namespace hidpg
{

HidReporter *Hid::_hid_reporter = nullptr;
uint8_t Hid::_pressed_keys[7] = {};
uint8_t Hid::_prev_sent_keys[6] = {};
uint8_t Hid::_key_counters[256] = {};
uint8_t Hid::_modifier_counters[8] = {};
int32_t Hid::_one_shot_modifier_counters[8] = {};
int32_t Hid::_triggered_one_shot_modifier_counters[8] = {};
uint8_t Hid::_prev_sent_modifier = 0;
uint8_t Hid::_prev_sent_button = 0;
uint8_t Hid::_button_counters[5] = {};

void Hid::setReporter(HidReporter *hid_reporter)
{
  _hid_reporter = hid_reporter;
}

void Hid::setKey(KeyCode key_code)
{
  uint8_t kc = static_cast<uint8_t>(key_code);

  _key_counters[kc]++;

  // すでに入ってるなら追加しない
  for (int i = 0; i < 6; i++)
  {
    if (_pressed_keys[i] == kc)
    {
      return;
    }
  }
  // 開いているスペースを探して追加
  for (int i = 0; i < 6; i++)
  {
    if (_pressed_keys[i] == 0)
    {
      _pressed_keys[i] = kc;
      return;
    }
  }
  // 満杯ならずらして末尾に追加
  memmove(_pressed_keys, _pressed_keys + 1, 5);
  _pressed_keys[5] = kc;
}

void Hid::unsetKey(KeyCode key_code)
{
  uint8_t kc = static_cast<uint8_t>(key_code);

  _key_counters[kc]--;

  if (_key_counters[kc] == 0)
  {
    int i = 0;
    for (; i < 6; i++)
    {
      if (_pressed_keys[i] == kc)
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
static void countUp(T (&counters)[SIZE], uint8_t bit8)
{
  for (size_t i = 0; i < SIZE; i++)
  {
    if (bitRead(bit8, i))
    {
      counters[i]++;
    }
  }
}

template <typename T, size_t SIZE>
static void countDown(T (&counters)[SIZE], uint8_t bit8)
{
  for (size_t i = 0; i < SIZE; i++)
  {
    if (bitRead(bit8, i))
    {
      counters[i]--;
    }
  }
}

void Hid::setModifier(Modifier modifier)
{
  uint8_t mod = static_cast<uint8_t>(modifier);
  countUp(_modifier_counters, mod);
}

void Hid::unsetModifier(Modifier modifier)
{
  uint8_t mod = static_cast<uint8_t>(modifier);
  countDown(_modifier_counters, mod);
}

void Hid::holdOneShotModifier(Modifier modifier)
{
  uint8_t mod = static_cast<uint8_t>(modifier);
  countUp(_one_shot_modifier_counters, mod);
}

void Hid::releaseOneShotModifier(Modifier modifier)
{
  uint8_t mod = static_cast<uint8_t>(modifier);
  countDown(_triggered_one_shot_modifier_counters, mod);
  sendKeyReport(false);
}

void Hid::sendKeyReport(bool trigger_one_shot)
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
  uint8_t modifier = 0;
  for (int i = 0; i < 8; i++)
  {
    if (_modifier_counters[i] > 0)
    {
      modifier |= bit(i);
    }
  }

  // 非one_shotなmodifierが新しく追加されたかどうか計算
  is_modifier_adding = ((modifier & ~_prev_sent_modifier) != 0);

  // one_shotは発動した後も押され続けている場合は機能し続ける、有るならばmodifierに追加
  for (int i = 0; i < 8; i++)
  {
    if (_triggered_one_shot_modifier_counters[i] > 0)
    {
      modifier |= bit(i);
    }
  }

  // one_shotをmodifierに追加
  if (trigger_one_shot)
  {
    for (int i = 0; i < 8; i++)
    {
      if (_one_shot_modifier_counters[i] > 0)
      {
        modifier |= bit(i);
        _triggered_one_shot_modifier_counters[i] += _one_shot_modifier_counters[i];
        _one_shot_modifier_counters[i] = 0;
      }
    }
  }

  // 前回のmodifierとの比較
  if (modifier != _prev_sent_modifier)
  {
    is_changed = true;
    // one_shotも考慮してmodifierが新しく追加されたかどうかを再計算
    is_modifier_adding = ((modifier & ~_prev_sent_modifier) != 0);
  }

  if (is_key_adding && is_modifier_adding)
  {
    // keyとmodifierが同時に追加された場合はmodifierキーを送ってからkeyを送る
    // 全く同じタイミングで送ると一部の環境で意図しない動きになる（windowsキーを使ったショートカットなど）
    if (_hid_reporter != nullptr)
    {
      _hid_reporter->keyboardReport(modifier, _prev_sent_keys);
      _hid_reporter->keyboardReport(modifier, _pressed_keys);
    }
  }
  else if (is_changed)
  {
    if (_hid_reporter != nullptr)
    {
      _hid_reporter->keyboardReport(modifier, _pressed_keys);
    }
  }

  // 次回用に保存
  if (is_changed)
  {
    memcpy(_prev_sent_keys, _pressed_keys, sizeof(_prev_sent_keys));
    _prev_sent_modifier = modifier;
  }
}

void Hid::consumerKeyPress(UsageCode usage_code)
{
  if (_hid_reporter != nullptr)
  {
    _hid_reporter->consumerReport(static_cast<uint16_t>(usage_code));
  }
}

void Hid::consumerKeyRelease()
{
  if (_hid_reporter != nullptr)
  {
    _hid_reporter->consumerReport(0);
  }
}

void Hid::mouseMove(int8_t x, int8_t y)
{
  if (_hid_reporter != nullptr)
  {
    _hid_reporter->mouseReport(_prev_sent_button, x, y, 0, 0);
  }
}

void Hid::mouseScroll(int8_t scroll, int8_t horiz)
{
  sendKeyReport(true);
  if (_hid_reporter != nullptr)
  {
    _hid_reporter->mouseReport(_prev_sent_button, 0, 0, scroll, horiz);
  }
  sendKeyReport(false);
}

void Hid::mouseButtonPress(MouseButton button)
{
  uint8_t btn = static_cast<uint8_t>(button);
  countUp(_button_counters, btn);
  sendKeyReport(true);
  sendMouseButtonReport();
}

void Hid::mouseButtonRelease(MouseButton button)
{
  uint8_t btn = static_cast<uint8_t>(button);
  countDown(_button_counters, btn);
  sendKeyReport(false);
  sendMouseButtonReport();
}

void Hid::sendMouseButtonReport()
{
  uint8_t button = 0;

  for (int i = 0; i < 5; i++)
  {
    if (_button_counters[i] > 0)
    {
      button |= bit(i);
    }
  }
  if (button != _prev_sent_button)
  {
    _prev_sent_button = button;
    if (_hid_reporter != nullptr)
    {
      _hid_reporter->mouseReport(button, 0, 0, 0, 0);
    }
  }
}

} // namespace hidpg
