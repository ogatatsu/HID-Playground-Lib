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

HidReporter *Hid::_hidReporter = nullptr;
uint8_t Hid::_pressedKeys[7] = {};
uint8_t Hid::_prevSentKeys[6] = {};
uint8_t Hid::_keyCounter[256] = {};
uint8_t Hid::_modifierCounter[8] = {};
int32_t Hid::_oneShotModifierCounter[8] = {};
int32_t Hid::_triggeredOneShotModifierCounter[8] = {};
uint8_t Hid::_prevSentModifier = 0;
uint8_t Hid::_prevSentButton = 0;
uint8_t Hid::_buttonCounter[5] = {};

void Hid::setReporter(HidReporter *hidReporter)
{
  _hidReporter = hidReporter;
}

void Hid::setKey(Keycode keycode)
{
  _keyCounter[static_cast<uint8_t>(keycode)]++;

  addKey(static_cast<uint8_t>(keycode));
}

void Hid::addKey(uint8_t keycode)
{
  // すでに入ってるなら追加しない
  for (int i = 0; i < 6; i++)
  {
    if (_pressedKeys[i] == keycode)
    {
      return;
    }
  }
  // 開いているスペースを探して追加
  for (int i = 0; i < 6; i++)
  {
    if (_pressedKeys[i] == 0)
    {
      _pressedKeys[i] = keycode;
      return;
    }
  }
  // 満杯ならずらして末尾に追加
  memmove(_pressedKeys, _pressedKeys + 1, 5);
  _pressedKeys[5] = keycode;
}

void Hid::unsetKey(Keycode keycode)
{
  _keyCounter[static_cast<uint8_t>(keycode)]--;
  if (_keyCounter[static_cast<uint8_t>(keycode)] == 0)
  {
    removeKey(static_cast<uint8_t>(keycode));
  }
}

void Hid::removeKey(uint8_t keycode)
{
  int i = 0;
  for (; i < 6; i++)
  {
    if (_pressedKeys[i] == keycode)
    {
      _pressedKeys[i] = 0;
      break;
    }
  }
  // 削除したスペースを埋めるためにずらしていく
  // _pressedKeys[6]は常に0が入っているので末尾には0が補充される
  for (; i < 6; i++)
  {
    _pressedKeys[i] = _pressedKeys[i + 1];
  }
}

template <typename T, size_t SIZE>
static void countUp(T (&counter)[SIZE], uint8_t bit8)
{
  for (size_t i = 0; i < SIZE; i++)
  {
    if (bitRead(bit8, i))
    {
      counter[i]++;
    }
  }
}

template <typename T, size_t SIZE>
static void countDown(T (&counter)[SIZE], uint8_t countableBit)
{
  for (size_t i = 0; i < SIZE; i++)
  {
    if (bitRead(countableBit, i))
    {
      counter[i]--;
    }
  }
}

void Hid::setModifier(Modifier modifier)
{
  countUp(_modifierCounter, static_cast<uint8_t>(modifier));
}

void Hid::unsetModifier(Modifier modifier)
{
  countDown(_modifierCounter, static_cast<uint8_t>(modifier));
}

void Hid::setOneShotModifier(Modifier modifier)
{
  countUp(_oneShotModifierCounter, static_cast<uint8_t>(modifier));
}

void Hid::releaseOneShotModifier(Modifier modifier)
{
  countDown(_triggeredOneShotModifierCounter, static_cast<uint8_t>(modifier));
  sendKeyReport(false);
}

void Hid::sendKeyReport(bool triggerOneShot)
{
  // 前回送ったreportと比較して変更があるか
  bool isChanged = false;
  // 新しくkeyもしくはmodifierが追加されたか、減った場合はfalseのまま
  bool isKeyAdding = false;
  bool isModifierAdding = false;

  // Normal Key
  // 前回の6keyとの比較
  if (memcmp(_prevSentKeys, _pressedKeys, sizeof(_prevSentKeys)) != 0)
  {
    isChanged = true;

    Set prev, current;
    prev.addAll(_prevSentKeys, 6);
    current.addAll(_pressedKeys, 6);
    isKeyAdding = ((current - prev).count() != 0);
  }

  // この変数にmodifierCountを計算して追加していく
  uint8_t modifier = 0;

  // Normal Modifier
  for (int i = 0; i < 8; i++)
  {
    if (_modifierCounter[i] > 0)
    {
      modifier |= bit(i);
    }
  }

  // 非OneShotなmodifierが新しく追加されたかどうか
  isModifierAdding = ((modifier & ~_prevSentModifier) != 0);

  // OneShot Modifier
  // OneShotは発動した後も押され続けている場合は機能し続ける
  for (int i = 0; i < 8; i++)
  {
    if (_triggeredOneShotModifierCounter[i] > 0)
    {
      modifier |= bit(i);
    }
  }

  if (triggerOneShot)
  {
    for (int i = 0; i < 8; i++)
    {
      if (_oneShotModifierCounter[i] > 0)
      {
        modifier |= bit(i);
        _triggeredOneShotModifierCounter[i] += _oneShotModifierCounter[i];
        _oneShotModifierCounter[i] = 0;
      }
    }
  }

  // 前回のmodifierとの比較
  if (modifier != _prevSentModifier)
  {
    isChanged = true;
    // OneShotも考慮してmodifierが新しく追加されたかどうかを再計算
    isModifierAdding = ((modifier & ~_prevSentModifier) != 0);
  }

  if (isKeyAdding && isModifierAdding)
  {
    // keyとmodifierが同時に追加された場合はmodifierキーを送ってからkeyを送る
    // 全く同じタイミングで送ると一部の環境で意図しない動きになる（windowsキーを使ったショートカットなど）
    if (_hidReporter != nullptr)
    {
      _hidReporter->keyboardReport(modifier, _prevSentKeys);
      _hidReporter->keyboardReport(modifier, _pressedKeys);
    }
  }
  else if (isChanged)
  {
    if (_hidReporter != nullptr)
    {
      _hidReporter->keyboardReport(modifier, _pressedKeys);
    }
  }

  // 次回用に保存
  if (isChanged)
  {
    memcpy(_prevSentKeys, _pressedKeys, sizeof(_prevSentKeys));
    _prevSentModifier = modifier;
  }
}

void Hid::consumerKeyPress(UsageCode usageCode)
{
  if (_hidReporter != nullptr)
  {
    _hidReporter->consumerReport(static_cast<uint16_t>(usageCode));
  }
}

void Hid::consumerKeyRelease()
{
  if (_hidReporter != nullptr)
  {
    _hidReporter->consumerReport(0);
  }
}

void Hid::mouseMove(int8_t x, int8_t y)
{
  if (_hidReporter != nullptr)
  {
    _hidReporter->mouseReport(_prevSentButton, x, y, 0, 0);
  }
}

void Hid::mouseScroll(int8_t scroll, int8_t horiz)
{
  sendKeyReport(true);
  if (_hidReporter != nullptr)
  {
    _hidReporter->mouseReport(_prevSentButton, 0, 0, scroll, horiz);
  }
  sendKeyReport(false);
}

void Hid::mouseButtonPress(MouseButton button)
{
  countUp(_buttonCounter, static_cast<uint8_t>(button));
  sendKeyReport(true);
  sendMouseButtonReport();
}

void Hid::mouseButtonRelease(MouseButton button)
{
  countDown(_buttonCounter, static_cast<uint8_t>(button));
  sendKeyReport(false);
  sendMouseButtonReport();
}

void Hid::sendMouseButtonReport()
{
  uint8_t button = 0;

  for (int i = 0; i < 5; i++)
  {
    if (_buttonCounter[i] > 0)
    {
      button |= bit(i);
    }
  }
  if (button != _prevSentButton)
  {
    _prevSentButton = button;
    if (_hidReporter != nullptr)
    {
      _hidReporter->mouseReport(button, 0, 0, 0, 0);
    }
  }
}

} // namespace hidpg
