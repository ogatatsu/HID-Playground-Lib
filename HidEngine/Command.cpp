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
#include "Tapper.h"
#include <Arduino.h>

namespace hidpg
{

/*------------------------------------------------------------------*/
/* Command
 *------------------------------------------------------------------*/
Command *Command::_lastPressedCommand = nullptr;

// constructor
Command::Command() : _parent(nullptr), _prevState(false)
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
uint8_t Command::press(uint8_t accrued)
{
  uint8_t result = 1;
  if (_prevState == false) //FALL
  {
    _lastPressedCommand = this;

    for (int i = 0; i < _listenerList().size(); i++)
    {
      Command *listener = _listenerList().get(i);
      if (getRootCommand(listener) != getRootCommand(this))
      {
        listener->onDifferentRootCommandPress();
      }
    }
    result = onPress(accrued);
  }
  _prevState = true;
  return result;
}

void Command::release()
{
  if (_prevState == true) //RISE
  {
    onRelease();
  }
  _prevState = false;
}

bool Command::isLastPressed()
{
  return _lastPressedCommand == this;
};

void Command::addEventListener_DifferentRootCommandPress()
{
  _listenerList().add(this);
}

/*------------------------------------------------------------------*/
/* NormalKey
 *------------------------------------------------------------------*/
NormalKey::NormalKey(Keycode keycode) : _keycode(keycode)
{
}

uint8_t NormalKey::onPress(uint8_t accrued)
{
  Hid::setKey(_keycode);
  Hid::sendKeyReport(true);
  return 1;
}

void NormalKey::onRelease()
{
  Hid::unsetKey(_keycode);
  Hid::sendKeyReport(false);
}

/*------------------------------------------------------------------*/
/* ModifierKey
 *------------------------------------------------------------------*/
ModifierKey::ModifierKey(Modifier modifier) : _modifier(modifier)
{
}

uint8_t ModifierKey::onPress(uint8_t accrued)
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
CombinationKey::CombinationKey(Modifier modifier, Keycode keycode) : _modifier(modifier), _keycode(keycode)
{
}

uint8_t CombinationKey::onPress(uint8_t accrued)
{
  Hid::setKey(_keycode);
  Hid::setModifier(_modifier);
  Hid::sendKeyReport(true);
  return 1;
}

void CombinationKey::onRelease()
{
  Hid::unsetKey(_keycode);
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

uint8_t ModifierTap::onPress(uint8_t accrued)
{
  Hid::setModifier(_modifier);
  return 1;
}

void ModifierTap::onRelease()
{
  Hid::unsetModifier(_modifier);
  if (this->isLastPressed())
  {
    CmdTapper::tap(_command);
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

uint8_t OneShotModifier::onPress(uint8_t accrued)
{
  Hid::setOneShotModifier(_modifier);
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

uint8_t Layering::onPress(uint8_t accrued)
{
  // 現在のレイヤーの状態を取得
  bool layerState[LAYER_SIZE];
  Layer::getState(layerState);

  _runningCommand = nullptr;

  // layerを上から舐めていってonのlayerを探す
  int i = LAYER_SIZE - 1;
  for (; i >= 0; i--)
  {
    if (layerState[i] == true)
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
    _runningCommand = _commands[i];
    break;
  }
  // 委託する
  uint8_t result = 1;
  if (_runningCommand != nullptr)
  {
    result = _runningCommand->press(accrued);
  }
  return result;
}

void Layering::onRelease()
{
  if (_runningCommand != nullptr)
  {
    _runningCommand->release();
  }
}

/*------------------------------------------------------------------*/
/* LayerTap
 *------------------------------------------------------------------*/
LayerTap::LayerTap(uint8_t layerNumber, Command *command)
    : _layerNumber(layerNumber), _command(command)
{
  _command->setParent(this);
}

uint8_t LayerTap::onPress(uint8_t accrued)
{
  Layer::on(_layerNumber);
  return 1;
}

void LayerTap::onRelease()
{
  Layer::off(_layerNumber);
  if (this->isLastPressed())
  {
    CmdTapper::tap(_command);
  }
}

/*------------------------------------------------------------------*/
/* ToggleLayer
 *------------------------------------------------------------------*/
ToggleLayer::ToggleLayer(uint8_t layerNumber) : _layerNumber(layerNumber)
{
}

uint8_t ToggleLayer::onPress(uint8_t accrued)
{
  Layer::toggle(_layerNumber);
  return 1;
}

/*------------------------------------------------------------------*/
/* SwitchLayer
 *------------------------------------------------------------------*/
SwitchLayer::SwitchLayer(uint8_t layerNumber) : _layerNumber(layerNumber)
{
}

uint8_t SwitchLayer::onPress(uint8_t accrued)
{
  Layer::on(_layerNumber);
  return 1;
}

void SwitchLayer::onRelease()
{
  Layer::off(_layerNumber);
}

/*------------------------------------------------------------------*/
/* OneShotLayer
 *------------------------------------------------------------------*/
OneShotLayer::OneShotLayer(uint8_t layerNumber) : _layerNumber(layerNumber)
{
}

uint8_t OneShotLayer::onPress(uint8_t accrued)
{
  Layer::setOneShot(_layerNumber);
  Layer::peekOneShot(_chainedOSL);
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    if (_chainedOSL[i])
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
    if (_chainedOSL[i])
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
    pairs[i].tapCommand->setParent(this);
    pairs[i].holdCommand->setParent(this);
  }
}

uint8_t TapDance::onPress(uint8_t accrued)
{
  if (_state == State::Unexecuted || _state == State::Tap_or_NextCommand)
  {
    _count++;
    _state = State::Unfixed;
    startTimer(TAPPING_TERM);
  }
  return 1;
}

void TapDance::onRelease()
{
  if (_state == State::FixedToHold)
  {
    _runningCommand->release();
    _state = State::Unexecuted;
  }
  else if (_state == State::Unfixed)
  {
    if (_count == _len - 1)
    {
      CmdTapper::tap(_pairs[_count].tapCommand);
      _count = -1;
      _state = State::Unexecuted;
    }
    else
    {
      _state = State::Tap_or_NextCommand;
      startTimer(TAPPING_TERM);
    }
  }
}

void TapDance::onTimer()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _runningCommand = _pairs[_count].holdCommand;
    _count = -1;
    _runningCommand->press();
  }
  else if (_state == State::Tap_or_NextCommand)
  {
    CmdTapper::tap(_pairs[_count].tapCommand);
    _count = -1;
    _state = State::Unexecuted;
  }
}

void TapDance::onDifferentRootCommandPress()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _runningCommand = _pairs[_count].holdCommand;
    _count = -1;
    _runningCommand->press();
  }
  else if (_state == State::Tap_or_NextCommand)
  {
    CmdTapper::tap(_pairs[_count].tapCommand);
    _count = -1;
    _state = State::Unexecuted;
  }
}

/*------------------------------------------------------------------*/
/* TapOrHold
 *------------------------------------------------------------------*/
TapOrHold::TapOrHold(Command *tapCommand, unsigned int ms, Command *holdCommand)
    : TimerMixin(), _ms(ms), _state(State::Unexecuted), _tapCommand(tapCommand), _holdCommand(holdCommand)
{
  _tapCommand->setParent(this);
  _holdCommand->setParent(this);
}

uint8_t TapOrHold::onPress(uint8_t accrued)
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
    CmdTapper::tap(_tapCommand);
    _state = State::Unexecuted;
    stopTimer();
  }
  else if (_state == State::FixedToHold)
  {
    _holdCommand->release();
    _state = State::Unexecuted;
  }
}

void TapOrHold::onTimer()
{
  if (_state == State::Unfixed)
  {
    _state = State::FixedToHold;
    _holdCommand->press();
  }
}

/*------------------------------------------------------------------*/
/* ConsumerControll
 *------------------------------------------------------------------*/
ConsumerControll::ConsumerControll(UsageCode usageCode) : _usageCode(usageCode)
{
}

uint8_t ConsumerControll::onPress(uint8_t accrued)
{
  Hid::consumerKeyPress(_usageCode);
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
    startTimer(MOUSEKEY_DELAY);
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
  startTimer(MOUSEKEY_INTERVAL);
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

uint8_t MouseMove::onPress(uint8_t accrued)
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

uint8_t MouseSpeed::onPress(uint8_t accrued)
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

uint8_t MouseScroll::onPress(uint8_t accrued)
{
  uint8_t possible = 127 / max(abs(_scroll), abs(_horiz));
  uint8_t times = min(accrued, possible);

  Hid::mouseScroll(_scroll * times, _horiz * times);
  return times;
}

/*------------------------------------------------------------------*/
/* MouseClick
 *------------------------------------------------------------------*/
MouseClick::MouseClick(MouseButton button) : _button(button)
{
}

uint8_t MouseClick::onPress(uint8_t accrued)
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
Macro::DownKey::DownKey(Keycode keycode) : _keycode(keycode)
{
}

unsigned int Macro::DownKey::apply()
{
  Hid::setKey(_keycode);
  Hid::sendKeyReport(true);
  return 0;
}

Macro::UpKey::UpKey(Keycode keycode) : _keycode(keycode)
{
}

unsigned int Macro::UpKey::apply()
{
  Hid::unsetKey(_keycode);
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

Macro::Macro(MacroCommand **mcommands, size_t len) : TimerMixin(), _mcommands(mcommands), _len(len), _isRunning(false)
{
}

uint8_t Macro::onPress(uint8_t accrued)
{
  if (_isRunning)
  {
    // do nothing.
  }
  else
  {
    _isRunning = true;
    _count = 0;
    startTimer(1);
  }
  return 1;
}

void Macro::onTimer()
{
  while (_count < _len)
  {
    unsigned int delay = _mcommands[_count++]->apply();
    if (delay != 0)
    {
      startTimer(delay);
      return;
    }
  }
  _isRunning = false;
  stopTimer();
}

/*------------------------------------------------------------------*/
/* If
 *------------------------------------------------------------------*/
If::If(bool (*func)(), Command *trueCommand, Command *falseCommand)
    : _func(func), _trueCommand(trueCommand), _falseCommand(falseCommand)
{
  _trueCommand->setParent(this);
  _falseCommand->setParent(this);
}

uint8_t If::onPress(uint8_t accrued)
{
  _runningCommand = _func() ? _trueCommand : _falseCommand;
  _runningCommand->press();
  return 1;
}

void If::onRelease()
{
  _runningCommand->release();
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

uint8_t Double::onPress(uint8_t accrued)
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
