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

#include "Command.h"
#include "HidCore.h"
#include "Layer.h"
#include "MouseSpeedController.h"
#include <Arduino.h>

namespace hidpg
{

/*------------------------------------------------------------------*/
/* Command
 *------------------------------------------------------------------*/
Command *Command::_last_pressed_command = nullptr;

// constructor
Command::Command() : _parent(nullptr), _prev_state(false)
{
}

// private function
static Command *getRootCommand(Command *command)
{
  while (command->getParent() != nullptr)
  {
    command = command->getParent();
  }
  return command;
}

// instance method
uint8_t Command::press(uint8_t accumulation)
{
  uint8_t result = 1;
  if (_prev_state == false) //FALL
  {
    _last_pressed_command = this;

    for (int i = 0; i < _listener_list().size(); i++)
    {
      Command *listener = _listener_list().get(i);
      if (getRootCommand(listener) != getRootCommand(this))
      {
        listener->onDifferentRootCommandPress();
      }
    }
    result = onPress(accumulation);
  }
  _prev_state = true;
  return result;
}

void Command::release()
{
  if (_prev_state == true) //RISE
  {
    onRelease();
  }
  _prev_state = false;
}

bool Command::isLastPressed()
{
  return _last_pressed_command == this;
};

void Command::addEventListener_DifferentRootCommandPress()
{
  _listener_list().add(this);
}

/*------------------------------------------------------------------*/
/* NormalKey
 *------------------------------------------------------------------*/
NormalKey::NormalKey(KeyCode key_code) : _key_code(key_code)
{
}

uint8_t NormalKey::onPress(uint8_t accumulation)
{
  Hid::setKey(_key_code);
  Hid::sendKeyReport(true);
  return 1;
}

void NormalKey::onRelease()
{
  Hid::unsetKey(_key_code);
  Hid::sendKeyReport(false);
}

/*------------------------------------------------------------------*/
/* ModifierKey
 *------------------------------------------------------------------*/
ModifierKey::ModifierKey(Modifier modifier) : _modifier(modifier)
{
}

uint8_t ModifierKey::onPress(uint8_t accumulation)
{
  Hid::setModifier(_modifier);
  Hid::sendKeyReport(true);
  return 1;
}

void ModifierKey::onRelease()
{
  Hid::unsetModifier(_modifier);
  Hid::sendKeyReport(false);
}

/*------------------------------------------------------------------*/
/* CombinationKey
 *------------------------------------------------------------------*/
CombinationKey::CombinationKey(Modifier modifier, KeyCode key_code) : _modifier(modifier), _key_code(key_code)
{
}

uint8_t CombinationKey::onPress(uint8_t accumulation)
{
  Hid::setKey(_key_code);
  Hid::setModifier(_modifier);
  Hid::sendKeyReport(true);
  return 1;
}

void CombinationKey::onRelease()
{
  Hid::unsetKey(_key_code);
  Hid::unsetModifier(_modifier);
  Hid::sendKeyReport(false);
}

/*------------------------------------------------------------------*/
/* ModifierTap
 *------------------------------------------------------------------*/
ModifierTap::ModifierTap(Modifier modifier, Command *command)
    : _modifier(modifier), _command(command)
{
  _command->setParent(this);
}

uint8_t ModifierTap::onPress(uint8_t accumulation)
{
  Hid::setModifier(_modifier);
  return 1;
}

void ModifierTap::onRelease()
{
  Hid::unsetModifier(_modifier);
  if (this->isLastPressed())
  {
    _command->press();
    _command->release();
  }
  else
  {
    Hid::sendKeyReport(false);
  }
}

/*------------------------------------------------------------------*/
/* OneShotModifier
 *------------------------------------------------------------------*/
OneShotModifier::OneShotModifier(Modifier modifier) : _modifier(modifier)
{
}

uint8_t OneShotModifier::onPress(uint8_t accumulation)
{
  Hid::holdOneShotModifier(_modifier);
  return 1;
}

void OneShotModifier::onRelease()
{
  Hid::releaseOneShotModifier(_modifier);
}

/*------------------------------------------------------------------*/
/* Layering
 *------------------------------------------------------------------*/
Layering::Layering(Command *commands[LAYER_SIZE]) : _commands(commands)
{
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    if (commands[i] != nullptr)
    {
      commands[i]->setParent(this);
    }
  }
}

uint8_t Layering::onPress(uint8_t accumulation)
{
  // 現在のレイヤーの状態を取得
  bool layer_state[LAYER_SIZE];
  Layer::getState(layer_state);

  _running_command = nullptr;

  // layerを上から舐めていってonのlayerを探す
  int i = LAYER_SIZE - 1;
  for (; i >= 0; i--)
  {
    if (layer_state[i] == true)
    {
      break;
    }
  }

  // onのレイヤーでもそのlayerのコマンドがnullptr(transparent)ならさらに下を探していく
  for (; i >= 0; i--)
  {
    if (_commands[i] == nullptr)
    {
      continue;
    }
    // 見つかった
    _running_command = _commands[i];
    break;
  }
  // 委託する
  uint8_t result = 1;
  if (_running_command != nullptr)
  {
    result = _running_command->press(accumulation);
  }
  return result;
}

void Layering::onRelease()
{
  if (_running_command != nullptr)
  {
    _running_command->release();
  }
}

/*------------------------------------------------------------------*/
/* LayerTap
 *------------------------------------------------------------------*/
LayerTap::LayerTap(uint8_t layer_number, Command *command)
    : _layer_number(layer_number), _command(command)
{
  _command->setParent(this);
}

uint8_t LayerTap::onPress(uint8_t accumulation)
{
  Layer::on(_layer_number);
  return 1;
}

void LayerTap::onRelease()
{
  Layer::off(_layer_number);
  if (this->isLastPressed())
  {
    _command->press();
    _command->release();
  }
}

/*------------------------------------------------------------------*/
/* ToggleLayer
 *------------------------------------------------------------------*/
ToggleLayer::ToggleLayer(uint8_t layer_number) : _layer_number(layer_number)
{
}

uint8_t ToggleLayer::onPress(uint8_t accumulation)
{
  Layer::toggle(_layer_number);
  return 1;
}

/*------------------------------------------------------------------*/
/* SwitchLayer
 *------------------------------------------------------------------*/
SwitchLayer::SwitchLayer(uint8_t layer_number) : _layer_number(layer_number)
{
}

uint8_t SwitchLayer::onPress(uint8_t accumulation)
{
  Layer::on(_layer_number);
  return 1;
}

void SwitchLayer::onRelease()
{
  Layer::off(_layer_number);
}

/*------------------------------------------------------------------*/
/* OneShotLayer
 *------------------------------------------------------------------*/
OneShotLayer::OneShotLayer(uint8_t layer_number) : _layer_number(layer_number)
{
}

uint8_t OneShotLayer::onPress(uint8_t accumulation)
{
  Layer::setOneShot(_layer_number);
  Layer::peekOneShot(_chained_osl);
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    if (_chained_osl[i])
    {
      Layer::on(i);
    }
  }
  return 1;
}

void OneShotLayer::onRelease()
{
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    if (_chained_osl[i])
    {
      Layer::off(i);
    }
  }
}

/*------------------------------------------------------------------*/
/* TapDance
 *------------------------------------------------------------------*/
TapDance::TapDance(Pair pairs[], size_t len)
    : TimerMixin(), _pairs(pairs), _len(len), _count(-1), _state(State::Unexecuted)
{
  addEventListener_DifferentRootCommandPress();
  for (size_t i = 0; i < len; i++)
  {
    pairs[i].tap_command->setParent(this);
    pairs[i].hold_command->setParent(this);
  }
}

uint8_t TapDance::onPress(uint8_t accumulation)
{
  if (_state == State::Unexecuted || _state == State::Tap_or_NextCommand)
  {
    _count++;
    _state = State::Unfixed;
    startTimer(TAPPING_TERM_MS);
  }
  return 1;
}

void TapDance::onRelease()
{
  if (_state == State::FixedToHold)
  {
    _running_command->release();
    _state = State::Unexecuted;
  }
  else if (_state == State::Unfixed)
  {
    if (_count == _len - 1)
    {
      _pairs[_count].tap_command->press();
      _pairs[_count].tap_command->release();
      _count = -1;
      _state = State::Unexecuted;
    }
    else
    {
      _state = State::Tap_or_NextCommand;
      startTimer(TAPPING_TERM_MS);
    }
  }
}

void TapDance::onTimer()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _running_command = _pairs[_count].hold_command;
    _count = -1;
    _running_command->press();
  }
  else if (_state == State::Tap_or_NextCommand)
  {
    _pairs[_count].tap_command->press();
    _pairs[_count].tap_command->release();
    _count = -1;
    _state = State::Unexecuted;
  }
}

void TapDance::onDifferentRootCommandPress()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _running_command = _pairs[_count].hold_command;
    _count = -1;
    _running_command->press();
  }
  else if (_state == State::Tap_or_NextCommand)
  {
    _pairs[_count].tap_command->press();
    _pairs[_count].tap_command->release();
    _count = -1;
    _state = State::Unexecuted;
  }
}

/*------------------------------------------------------------------*/
/* TapOrHold
 *------------------------------------------------------------------*/
TapOrHold::TapOrHold(Command *tap_command, unsigned int ms, Command *hold_command)
    : TimerMixin(), _ms(ms), _state(State::Unexecuted), _tap_command(tap_command), _hold_command(hold_command)
{
  _tap_command->setParent(this);
  _hold_command->setParent(this);
}

uint8_t TapOrHold::onPress(uint8_t accumulation)
{
  if (_state == State::Unexecuted)
  {
    _state = State::Unfixed;
    startTimer(_ms);
  }
  return 1;
}

void TapOrHold::onRelease()
{
  if (_state == State::Unfixed)
  {
    _tap_command->press();
    _tap_command->release();
    _state = State::Unexecuted;
    stopTimer();
  }
  else if (_state == State::FixedToHold)
  {
    _hold_command->release();
    _state = State::Unexecuted;
  }
}

void TapOrHold::onTimer()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _hold_command->press();
  }
}

/*------------------------------------------------------------------*/
/* ConsumerControll
 *------------------------------------------------------------------*/
ConsumerControll::ConsumerControll(UsageCode usage_code) : _usage_code(usage_code)
{
}

uint8_t ConsumerControll::onPress(uint8_t accumulation)
{
  Hid::consumerKeyPress(_usage_code);
  return 1;
}
void ConsumerControll::onRelease()
{
  Hid::consumerKeyRelease();
}

/*------------------------------------------------------------------*/
/* MouseMove
 *------------------------------------------------------------------*/
MouseMove::Mover::Mover() : TimerMixin(), _x(0), _y(0), _count(0)
{
}

void MouseMove::Mover::setXY(int8_t x, int8_t y)
{
  _count++;
  _x += x;
  _y += y;
  calcXY(x, y);
  Hid::mouseMove(x, y);
  if (_count == 1)
  {
    startTimer(MOUSEKEY_DELAY_MS);
  }
}

void MouseMove::Mover::unsetXY(int8_t x, int8_t y)
{
  _count--;
  _x -= x;
  _y -= y;
  if (_count == 0)
  {
    stopTimer();
  }
}

void MouseMove::Mover::onTimer()
{
  int8_t x, y;
  calcXY(x, y);
  Hid::mouseMove(x, y);
  startTimer(MOUSEKEY_INTERVAL_MS);
}

void MouseMove::Mover::calcXY(int8_t &x, int8_t &y)
{
  double factor = MouseSpeedController::getfactor();
  int ix, iy;
  ix = round(_x * factor);
  ix = constrain(ix, -127, 127);
  iy = round(_y * factor);
  iy = constrain(iy, -127, 127);
  x = static_cast<int8_t>(ix);
  y = static_cast<int8_t>(iy);
}

MouseMove::Mover MouseMove::_mover;

MouseMove::MouseMove(int8_t x, int8_t y)
    : _x(x), _y(y)
{
}

uint8_t MouseMove::onPress(uint8_t accumulation)
{
  _mover.setXY(_x, _y);
  return 1;
}

void MouseMove::onRelease()
{
  _mover.unsetXY(_x, _y);
}

/*------------------------------------------------------------------*/
/* MouseSpeed
 *------------------------------------------------------------------*/
MouseSpeed::MouseSpeed(int16_t percent)
    : _percent(percent)
{
}

uint8_t MouseSpeed::onPress(uint8_t accumulation)
{
  if (_percent == 0)
  {
    MouseSpeedController::setZero();
  }
  else
  {
    MouseSpeedController::accel(_percent);
  }
  return 1;
}

void MouseSpeed::onRelease()
{
  if (_percent == 0)
  {
    MouseSpeedController::unsetZero();
  }
  else
  {
    MouseSpeedController::decel(_percent);
  }
}

/*------------------------------------------------------------------*/
/* MouseScroll
 *------------------------------------------------------------------*/
MouseScroll::MouseScroll(int8_t scroll, int8_t horiz) : _scroll(scroll), _horiz(horiz)
{
}

uint8_t MouseScroll::onPress(uint8_t accumulation)
{
  uint8_t possible = 127 / max(abs(_scroll), abs(_horiz));
  uint8_t times = min(accumulation, possible);

  Hid::mouseScroll(_scroll * times, _horiz * times);
  return times;
}

/*------------------------------------------------------------------*/
/* MouseClick
 *------------------------------------------------------------------*/
MouseClick::MouseClick(MouseButton button) : _button(button)
{
}

uint8_t MouseClick::onPress(uint8_t accumulation)
{
  Hid::mouseButtonPress(_button);
  return 1;
}

void MouseClick::onRelease()
{
  Hid::mouseButtonRelease(_button);
}

/*------------------------------------------------------------------*/
/* Macro
 *------------------------------------------------------------------*/
Macro::DownKey::DownKey(KeyCode key_code) : _key_code(key_code)
{
}

unsigned int Macro::DownKey::apply()
{
  Hid::setKey(_key_code);
  Hid::sendKeyReport(true);
  return 0;
}

Macro::UpKey::UpKey(KeyCode key_code) : _key_code(key_code)
{
}

unsigned int Macro::UpKey::apply()
{
  Hid::unsetKey(_key_code);
  Hid::sendKeyReport(false);
  return 0;
}

Macro::DownModifier::DownModifier(Modifier modifier) : _modifier(modifier)
{
}

unsigned int Macro::DownModifier::apply()
{
  Hid::setModifier(_modifier);
  Hid::sendKeyReport(true);
  return 0;
}

Macro::UpModifier::UpModifier(Modifier modifier) : _modifier(modifier)
{
}

unsigned int Macro::UpModifier::apply()
{
  Hid::unsetModifier(_modifier);
  Hid::sendKeyReport(false);
  return 0;
}

Macro::Wait::Wait(unsigned int delay) : _delay(delay)
{
}

unsigned int Macro::Wait::apply()
{
  return _delay;
}

Macro::Macro(MacroCommand **m_commands, size_t len) : TimerMixin(), _m_commands(m_commands), _len(len), _is_running(false)
{
}

uint8_t Macro::onPress(uint8_t accumulation)
{
  if (_is_running)
  {
    // do nothing.
  }
  else
  {
    _is_running = true;
    _count = 0;
    startTimer(1);
  }
  return 1;
}

void Macro::onTimer()
{
  while (_count < _len)
  {
    unsigned int delay = _m_commands[_count++]->apply();
    if (delay != 0)
    {
      startTimer(delay);
      return;
    }
  }
  _is_running = false;
  stopTimer();
}

/*------------------------------------------------------------------*/
/* If
 *------------------------------------------------------------------*/
If::If(bool (*func)(), Command *true_command, Command *false_command)
    : _func(func), _true_command(true_command), _false_command(false_command)
{
  _true_command->setParent(this);
  _false_command->setParent(this);
}

uint8_t If::onPress(uint8_t accumulation)
{
  _running_command = _func() ? _true_command : _false_command;
  _running_command->press();
  return 1;
}

void If::onRelease()
{
  _running_command->release();
}

/*------------------------------------------------------------------*/
/* Double
 *------------------------------------------------------------------*/
Double::Double(Command *command1, Command *command2)
    : _command1(command1), _command2(command2)
{
  _command1->setParent(this);
  _command2->setParent(this);
}

uint8_t Double::onPress(uint8_t accumulation)
{
  _command1->press();
  _command2->press();
  return 1;
}

void Double::onRelease()
{
  _command2->release();
  _command1->release();
}

} // namespace hidpg
