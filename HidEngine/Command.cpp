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
#include "Arduino.h"
#include "HidCore.h"
#include "MouseSpeedController.h"

namespace hidpg
{

  //------------------------------------------------------------------+
  // Command
  //------------------------------------------------------------------+
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
  void Command::press(uint8_t n_times)
  {
    if (_prev_state == false) //FALL
    {
      _last_pressed_command = this;

      // notify before different command press
      for (int i = 0; i < _listener_list().size(); i++)
      {
        Command *listener = _listener_list().get(i);
        if (getRootCommand(listener) != getRootCommand(this))
        {
          listener->onBeforeInput();
        }
      }
      onPress(n_times);
    }
    _prev_state = true;
  }

  uint8_t Command::release()
  {
    uint8_t result = 0;

    if (_prev_state == true) //RISE
    {
      result = onRelease();
    }
    _prev_state = false;

    return result;
  }

  bool Command::isLastPressed()
  {
    return _last_pressed_command == this;
  }

  void Command::addEventListener_BeforeInput()
  {
    _listener_list().add(this);
  }

  void Command::_notifyBeforeMouseMove()
  {
    for (int i = 0; i < _listener_list().size(); i++)
    {
      Command *listener = _listener_list().get(i);
      listener->onBeforeInput();
    }
  }

  //------------------------------------------------------------------+
  // NormalKey
  //------------------------------------------------------------------+
  NormalKey::NormalKey(KeyCode key_code) : _key_code(key_code)
  {
  }

  void NormalKey::onPress(uint8_t n_times)
  {
    Hid.setKey(_key_code);
    Hid.sendKeyReport(true);
  }

  uint8_t NormalKey::onRelease()
  {
    Hid.unsetKey(_key_code);
    Hid.sendKeyReport(false);
    return 1;
  }

  //------------------------------------------------------------------+
  // ModifierKey
  //------------------------------------------------------------------+
  ModifierKey::ModifierKey(Modifier modifier) : _modifier(modifier)
  {
  }

  void ModifierKey::onPress(uint8_t n_times)
  {
    Hid.setModifier(_modifier);
    Hid.sendKeyReport(true);
  }

  uint8_t ModifierKey::onRelease()
  {
    Hid.unsetModifier(_modifier);
    Hid.sendKeyReport(false);
    return 1;
  }

  //------------------------------------------------------------------+
  // CombinationKey
  //------------------------------------------------------------------+
  CombinationKey::CombinationKey(Modifier modifier, KeyCode key_code) : _modifier(modifier), _key_code(key_code)
  {
  }

  void CombinationKey::onPress(uint8_t n_times)
  {
    Hid.setKey(_key_code);
    Hid.setModifier(_modifier);
    Hid.sendKeyReport(true);
  }

  uint8_t CombinationKey::onRelease()
  {
    Hid.unsetKey(_key_code);
    Hid.unsetModifier(_modifier);
    Hid.sendKeyReport(false);
    return 1;
  }

  //------------------------------------------------------------------+
  // ModifierTap
  //------------------------------------------------------------------+
  ModifierTap::ModifierTap(Modifier modifier, Command *command)
      : _modifier(modifier), _command(command)
  {
    _command->setParent(this);
  }

  void ModifierTap::onPress(uint8_t n_times)
  {
    Hid.setModifier(_modifier);
  }

  uint8_t ModifierTap::onRelease()
  {
    Hid.unsetModifier(_modifier);
    if (this->isLastPressed())
    {
      _command->press();
      _command->release();
    }
    else
    {
      Hid.sendKeyReport(false);
    }
    return 1;
  }

  //------------------------------------------------------------------+
  // OneShotModifier
  //------------------------------------------------------------------+
  OneShotModifier::OneShotModifier(Modifier modifier) : _modifier(modifier)
  {
  }

  void OneShotModifier::onPress(uint8_t n_times)
  {
    Hid.holdOneShotModifier(_modifier);
  }

  uint8_t OneShotModifier::onRelease()
  {
    Hid.releaseOneShotModifier(_modifier);
    return 1;
  }

  //------------------------------------------------------------------+
  // Layering
  //------------------------------------------------------------------+
  Layering::Layering(LayerClass *layer, Command *commands[HID_ENGINE_LAYER_SIZE]) : _layer(layer), _commands(commands)
  {
    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      if (commands[i] != nullptr)
      {
        commands[i]->setParent(this);
      }
    }
  }

  void Layering::onPress(uint8_t n_times)
  {
    // 現在のレイヤーの状態を取得
    bool layer_state[HID_ENGINE_LAYER_SIZE];
    _layer->takeState(layer_state);

    _running_command = nullptr;

    // layerを上から舐めていってonのlayerを探す
    int i = HID_ENGINE_LAYER_SIZE - 1;
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
    if (_running_command != nullptr)
    {
      _running_command->press(n_times);
    }
  }

  uint8_t Layering::onRelease()
  {
    uint8_t result = 1;
    if (_running_command != nullptr)
    {
      result = _running_command->release();
    }
    return result;
  }

  //------------------------------------------------------------------+
  // LayerTap
  //------------------------------------------------------------------+
  LayerTap::LayerTap(LayerClass *layer, uint8_t layer_number, Command *command)
      : _layer(layer), _layer_number(layer_number), _command(command)
  {
    _command->setParent(this);
  }

  void LayerTap::onPress(uint8_t n_times)
  {
    _layer->on(_layer_number);
  }

  uint8_t LayerTap::onRelease()
  {
    _layer->off(_layer_number);
    if (this->isLastPressed())
    {
      _command->press();
      _command->release();
    }
    return 1;
  }

  //------------------------------------------------------------------+
  // ToggleLayer
  //------------------------------------------------------------------+
  ToggleLayer::ToggleLayer(LayerClass *layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void ToggleLayer::onPress(uint8_t n_times)
  {
    _layer->toggle(_layer_number);
  }

  //------------------------------------------------------------------+
  // SwitchLayer
  //------------------------------------------------------------------+
  SwitchLayer::SwitchLayer(LayerClass *layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void SwitchLayer::onPress(uint8_t n_times)
  {
    _layer->on(_layer_number);
  }

  uint8_t SwitchLayer::onRelease()
  {
    _layer->off(_layer_number);
    return 1;
  }

  //------------------------------------------------------------------+
  // OneShotLayer
  //------------------------------------------------------------------+
  OneShotLayer::OneShotLayer(LayerClass *layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void OneShotLayer::onPress(uint8_t n_times)
  {
    _layer->setOneShot(_layer_number);
    _layer->peekOneShot(_chained_osl);
    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      if (_chained_osl[i])
      {
        _layer->on(i);
      }
    }
  }

  uint8_t OneShotLayer::onRelease()
  {
    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      if (_chained_osl[i])
      {
        _layer->off(i);
      }
    }
    return 1;
  }

  //------------------------------------------------------------------+
  // Tap
  //------------------------------------------------------------------+
  Tap::Tap(Command *command, uint8_t n_times) : _command(command), _n_times(n_times)
  {
    _command->setParent(this);
  }

  void Tap::onPress(uint8_t n_times)
  {
    for (int i = 0; i < _n_times; i++)
    {
      _command->press();
      _command->release();
    }
  }

  //------------------------------------------------------------------+
  // TapDance
  //------------------------------------------------------------------+
  TapDance::TapDance(Pair pairs[], size_t len)
      : TimerMixin(), _pairs(pairs), _len(len), _count(-1), _state(State::Unexecuted)
  {
    addEventListener_BeforeInput();
    for (size_t i = 0; i < len; i++)
    {
      pairs[i].tap_command->setParent(this);
      pairs[i].hold_command->setParent(this);
    }
  }

  void TapDance::onPress(uint8_t n_times)
  {
    if (_state == State::Unexecuted || _state == State::Tap_or_NextCommand)
    {
      _count++;
      _state = State::Unfixed;
      startTimer(HID_ENGINE_TAPPING_TERM_MS);
    }
  }

  uint8_t TapDance::onRelease()
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
        startTimer(HID_ENGINE_TAPPING_TERM_MS);
      }
    }
    return 1;
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

  void TapDance::onBeforeInput()
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

  //------------------------------------------------------------------+
  // TapOrHold
  //------------------------------------------------------------------+
  TapOrHold::TapOrHold(Command *tap_command, unsigned int ms, Command *hold_command)
      : TimerMixin(), _ms(ms), _state(State::Unexecuted), _tap_command(tap_command), _hold_command(hold_command)
  {
    _tap_command->setParent(this);
    _hold_command->setParent(this);
  }

  void TapOrHold::onPress(uint8_t n_times)
  {
    if (_state == State::Unexecuted)
    {
      _state = State::Unfixed;
      startTimer(_ms);
    }
  }

  uint8_t TapOrHold::onRelease()
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
    return 1;
  }

  void TapOrHold::onTimer()
  {
    if (_state == State::Unfixed)
    {
      _state = State::FixedToHold;
      _hold_command->press();
    }
  }

  //------------------------------------------------------------------+
  // ConsumerControll
  //------------------------------------------------------------------+
  ConsumerControll::ConsumerControll(UsageCode usage_code) : _usage_code(usage_code)
  {
  }

  void ConsumerControll::onPress(uint8_t n_times)
  {
    Hid.consumerKeyPress(_usage_code);
  }
  uint8_t ConsumerControll::onRelease()
  {
    Hid.consumerKeyRelease();
    return 1;
  }

  //------------------------------------------------------------------+
  // MouseMove
  //------------------------------------------------------------------+
  MouseMove::Mover::Mover() : TimerMixin(), _total_x(0), _total_y(0), _count(0)
  {
  }

  void MouseMove::Mover::setXY(int16_t x, int16_t y)
  {
    _count++;
    _total_x += x;
    _total_y += y;
    calcXY(x, y);
    Hid.mouseMove(x, y);
    if (_count == 1)
    {
      startTimer(HID_ENGINE_MOUSEKEY_DELAY_MS);
    }
  }

  void MouseMove::Mover::unsetXY(int16_t x, int16_t y)
  {
    _count--;
    _total_x -= x;
    _total_y -= y;
    if (_count == 0)
    {
      stopTimer();
    }
  }

  void MouseMove::Mover::onTimer()
  {
    int16_t x, y;
    calcXY(x, y);
    Hid.mouseMove(x, y);
    startTimer(HID_ENGINE_MOUSEKEY_INTERVAL_MS);
  }

  void MouseMove::Mover::calcXY(int16_t &x, int16_t &y)
  {
    double factor = MouseSpeedController.getfactor();
    int ix, iy;
    ix = round(_total_x * factor);
    ix = constrain(ix, INT16_MIN, INT16_MAX);
    iy = round(_total_y * factor);
    iy = constrain(iy, INT16_MIN, INT16_MAX);
    x = static_cast<int16_t>(ix);
    y = static_cast<int16_t>(iy);
  }

  MouseMove::Mover MouseMove::_mover;

  MouseMove::MouseMove(int16_t x, int16_t y)
      : _x(x), _y(y)
  {
  }

  void MouseMove::onPress(uint8_t n_times)
  {
    _mover.setXY(_x, _y);
  }

  uint8_t MouseMove::onRelease()
  {
    _mover.unsetXY(_x, _y);
    return 1;
  }

  //------------------------------------------------------------------+
  // MouseSpeed
  //------------------------------------------------------------------+
  MouseSpeed::MouseSpeed(int16_t percent)
      : _percent(percent)
  {
  }

  void MouseSpeed::onPress(uint8_t n_times)
  {
    if (_percent == 0)
    {
      MouseSpeedController.setZero();
    }
    else
    {
      MouseSpeedController.accel(_percent);
    }
  }

  uint8_t MouseSpeed::onRelease()
  {
    if (_percent == 0)
    {
      MouseSpeedController.unsetZero();
    }
    else
    {
      MouseSpeedController.decel(_percent);
    }
    return 1;
  }

  //------------------------------------------------------------------+
  // MouseScroll
  //------------------------------------------------------------------+
  MouseScroll::MouseScroll(int8_t scroll, int8_t horiz) : _scroll(scroll), _horiz(horiz)
  {
  }

  void MouseScroll::onPress(uint8_t n_times)
  {
    uint8_t possible = 127 / max(abs(_scroll), abs(_horiz));
    possible = min(n_times, possible);

    Hid.mouseScroll(_scroll * possible, _horiz * possible);
    _n_times = possible;
  }

  uint8_t MouseScroll::onRelease()
  {
    return _n_times;
  }

  //------------------------------------------------------------------+
  // MouseClick
  //------------------------------------------------------------------+
  MouseClick::MouseClick(MouseButton button) : _button(button)
  {
  }

  void MouseClick::onPress(uint8_t n_times)
  {
    Hid.mouseButtonPress(_button);
  }

  uint8_t MouseClick::onRelease()
  {
    Hid.mouseButtonRelease(_button);
    return 1;
  }

  //------------------------------------------------------------------+
  // If
  //------------------------------------------------------------------+
  If::If(bool (*func)(), Command *true_command, Command *false_command)
      : _func(func), _true_command(true_command), _false_command(false_command)
  {
    _true_command->setParent(this);
    _false_command->setParent(this);
  }

  void If::onPress(uint8_t n_times)
  {
    _running_command = _func() ? _true_command : _false_command;
    _running_command->press();
  }

  uint8_t If::onRelease()
  {
    return _running_command->release();
  }

  //------------------------------------------------------------------+
  // Double
  //------------------------------------------------------------------+
  Double::Double(Command *command1, Command *command2)
      : _command1(command1), _command2(command2)
  {
    _command1->setParent(this);
    _command2->setParent(this);
  }

  void Double::onPress(uint8_t n_times)
  {
    _command1->press();
    _command2->press();
  }

  uint8_t Double::onRelease()
  {
    _command2->release();
    _command1->release();
    return 1;
  }

} // namespace hidpg
