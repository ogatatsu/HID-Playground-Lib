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
#include "ArduinoMacro.h"
#include "CommandTapper.h"
#include "HidCore.h"
#include "MouseSpeedController.h"

extern "C"
{
  uint32_t millis();
}

namespace hidpg
{

  //------------------------------------------------------------------+
  // Command
  //------------------------------------------------------------------+
  Command *Command::_last_pressed_command = nullptr;

  Command::Command() : _parent(nullptr), _prev_state(false)
  {
  }

  void Command::press(uint8_t n_times)
  {
    if (_prev_state == false) // FALL
    {
      BdrcpEventListener::_notifyBeforeDifferentRootCommandPress(*this);

      _last_pressed_command = this;
      onPress(n_times);
    }
    _prev_state = true;
  }

  uint8_t Command::release()
  {
    uint8_t result = 0;

    if (_prev_state == true) // RISE
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
  //------------------------------------------------------------------+
  // BdrcpEventListener
  //------------------------------------------------------------------+
  BdrcpEventListener::BdrcpEventListener(Command *command) : _command(command), _is_listen(false)
  {
  }

  void BdrcpEventListener::startListen_BeforeDifferentRootCommandPress()
  {
    if (_is_listen == false)
    {
      _listener_list().push_back(*this);
      _is_listen = true;
    }
  }

  void BdrcpEventListener::stopListen_BeforeDifferentRootCommandPress()
  {
    if (_is_listen)
    {
      auto i_item = etl::intrusive_list<BdrcpEventListener, BdrcpEventListenerLink>::iterator(*this);
      _listener_list().erase(i_item);
      _is_listen = false;
    }
  }

  void BdrcpEventListener::_notifyBeforeDifferentRootCommandPress(Command &press_command)
  {
    for (BdrcpEventListener &listener : _listener_list())
    {
      if (getRootCommand(listener._command) != getRootCommand(&press_command))
      {
        listener.onBeforeDifferentRootCommandPress();
      }
    }
  }

  Command *BdrcpEventListener::getRootCommand(Command *command)
  {
    while (command->getParent() != nullptr)
    {
      command = command->getParent();
    }
    return command;
  }

  //------------------------------------------------------------------+
  // BmmEventListener
  //------------------------------------------------------------------+
  BmmEventListener::BmmEventListener() : _is_listen(false)
  {
  }

  void BmmEventListener::startListen_BeforeMouseMove()
  {
    if (_is_listen == false)
    {
      _listener_list().push_back(*this);
      _is_listen = true;
    }
  }

  void BmmEventListener::stopListen_BeforeMouseMove()
  {
    if (_is_listen)
    {
      auto i_item = etl::intrusive_list<BmmEventListener, BmmEventListenerLink>::iterator(*this);
      _listener_list().erase(i_item);
      _is_listen = false;
    }
  }

  void BmmEventListener::_notifyBeforeMouseMove(uint8_t mouse_id)
  {
    for (BmmEventListener &listener : _listener_list())
    {
      listener.onBeforeMouseMove(mouse_id);
    }
  }

  //------------------------------------------------------------------+
  // BgstEventListener
  //------------------------------------------------------------------+
  BgstEventListener::BgstEventListener() : _is_listen(false)
  {
  }

  void BgstEventListener::startListen_BeforeGesture()
  {
    if (_is_listen == false)
    {
      _listener_list().push_back(*this);
      _is_listen = true;
    }
  }

  void BgstEventListener::stopListen_BeforeGesture()
  {
    if (_is_listen)
    {
      auto i_item = etl::intrusive_list<BgstEventListener, BgstEventListenerLink>::iterator(*this);
      _listener_list().erase(i_item);
      _is_listen = false;
    }
  }

  void BgstEventListener::_notifyBeforeGesture(uint8_t gesture_id, uint8_t mouse_id)
  {
    for (BgstEventListener &listener : _listener_list())
    {
      listener.onBeforeGesture(gesture_id, mouse_id);
    }
  }

  namespace Internal
  {

    //------------------------------------------------------------------+
    // NormalKey
    //------------------------------------------------------------------+
    NormalKey::NormalKey(KeyCode key_code) : _key_code(key_code)
    {
    }

    void NormalKey::onPress(uint8_t n_times)
    {
      Hid.setKey(_key_code);
      Hid.sendKeyReport();
    }

    uint8_t NormalKey::onRelease()
    {
      Hid.unsetKey(_key_code);
      Hid.sendKeyReport();
      return 1;
    }

    //------------------------------------------------------------------+
    // ModifierKey
    //------------------------------------------------------------------+
    ModifierKey::ModifierKey(Modifiers modifiers) : _modifiers(modifiers)
    {
    }

    void ModifierKey::onPress(uint8_t n_times)
    {
      Hid.setModifiers(_modifiers);
      Hid.sendKeyReport();
    }

    uint8_t ModifierKey::onRelease()
    {
      Hid.unsetModifiers(_modifiers);
      Hid.sendKeyReport();
      return 1;
    }

    //------------------------------------------------------------------+
    // CombinationKey
    //------------------------------------------------------------------+
    CombinationKey::CombinationKey(Modifiers modifiers, KeyCode key_code) : _modifiers(modifiers), _key_code(key_code)
    {
    }

    void CombinationKey::onPress(uint8_t n_times)
    {
      Hid.setKey(_key_code);
      Hid.setModifiers(_modifiers);
      Hid.sendKeyReport();
    }

    uint8_t CombinationKey::onRelease()
    {
      Hid.unsetKey(_key_code);
      Hid.unsetModifiers(_modifiers);
      Hid.sendKeyReport();
      return 1;
    }

    //------------------------------------------------------------------+
    // ModifierOrTap
    //------------------------------------------------------------------+
    ModifierOrTap::ModifierOrTap(Modifiers modifiers, Command *command)
        : _modifiers(modifiers), _command(command)
    {
      _command->setParent(this);
    }

    void ModifierOrTap::onPress(uint8_t n_times)
    {
      Hid.setModifiers(_modifiers);
    }

    uint8_t ModifierOrTap::onRelease()
    {
      Hid.unsetModifiers(_modifiers);
      if (this->isLastPressed())
      {
        CommandTapper.tap(_command);
      }
      else
      {
        Hid.sendKeyReport();
      }
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
      layer_bitmap_t layer_state = _layer->getState();

      _running_command = nullptr;

      // layerを上から舐めていってonのlayerを探す
      int i = HID_ENGINE_LAYER_SIZE - 1;
      for (; i >= 0; i--)
      {
        if (bitRead(layer_state, i) == 1)
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
    // LayerOrTap
    //------------------------------------------------------------------+
    LayerOrTap::LayerOrTap(LayerClass *layer, uint8_t layer_number, Command *command)
        : _layer(layer), _layer_number(layer_number), _command(command)
    {
      _command->setParent(this);
    }

    void LayerOrTap::onPress(uint8_t n_times)
    {
      _layer->on(_layer_number);
    }

    uint8_t LayerOrTap::onRelease()
    {
      _layer->off(_layer_number);
      if (this->isLastPressed())
      {
        CommandTapper.tap(_command);
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
    // UpBaseLayer
    //------------------------------------------------------------------+
    UpBaseLayer::UpBaseLayer(LayerClass *layer, uint8_t i) : _layer(layer), _i(i)
    {
    }

    void UpBaseLayer::onPress(uint8_t n_times)
    {
      _layer->addToBase(_i);
    }

    uint8_t UpBaseLayer::onRelease()
    {
      _layer->addToBase(-_i);
      return 1;
    }

    //------------------------------------------------------------------+
    // Tap
    //------------------------------------------------------------------+
    Tap::Tap(Command *command, uint8_t n_times, uint16_t tap_speed_ms)
        : _command(command), _n_times(n_times), _tap_speed_ms(tap_speed_ms)
    {
      _command->setParent(this);
    }

    void Tap::onPress(uint8_t n_times)
    {
      CommandTapper.tap(_command, _n_times, _tap_speed_ms);
    }

    //------------------------------------------------------------------+
    // TapWhenReleased
    //------------------------------------------------------------------+
    TapWhenReleased::TapWhenReleased(Command *command, uint8_t n_times, uint16_t tap_speed_ms)
        : _command(command), _n_times(n_times), _tap_speed_ms(tap_speed_ms)
    {
      _command->setParent(this);
    }

    uint8_t TapWhenReleased::onRelease()
    {
      CommandTapper.tap(_command, _n_times, _tap_speed_ms);
      return 1;
    }

    //------------------------------------------------------------------+
    // TapDance
    //------------------------------------------------------------------+
    TapDance::TapDance(Pair pairs[], uint8_t len)
        : TimerMixin(), BdrcpEventListener(this),
          _pairs(pairs), _len(len), _idx_count(-1), _state(State::Unexecuted)
    {
      for (int i = 0; i < len; i++)
      {
        pairs[i].tap_command->setParent(this);
        pairs[i].hold_command->setParent(this);
      }
    }

    void TapDance::onPress(uint8_t n_times)
    {
      if (_state == State::Unexecuted)
      {
        startListen_BeforeDifferentRootCommandPress();
      }

      if (_state == State::Unexecuted || _state == State::Tap_or_NextCommand)
      {
        _idx_count++;
        _state = State::Pressed;
        startTimer(HID_ENGINE_TAPPING_TERM_MS);
      }
    }

    uint8_t TapDance::onRelease()
    {
      if (_state == State::FixedToHold)
      {
        _running_command->release();
        _state = State::Unexecuted;
        stopListen_BeforeDifferentRootCommandPress();
      }
      else if (_state == State::Pressed)
      {
        if (_idx_count == _len - 1)
        {
          CommandTapper.tap(_pairs[_idx_count].tap_command);
          _idx_count = -1;
          _state = State::Unexecuted;
          stopListen_BeforeDifferentRootCommandPress();
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
      if (_state == State::Pressed)
      {
        _state = State::FixedToHold;
        _running_command = _pairs[_idx_count].hold_command;
        _idx_count = -1;
        _running_command->press();
      }
      else if (_state == State::Tap_or_NextCommand)
      {
        CommandTapper.tap(_pairs[_idx_count].tap_command);
        _idx_count = -1;
        _state = State::Unexecuted;
        stopListen_BeforeDifferentRootCommandPress();
      }
    }

    void TapDance::onBeforeDifferentRootCommandPress()
    {
      if (_state == State::Pressed)
      {
        _state = State::FixedToHold;
        _running_command = _pairs[_idx_count].hold_command;
        _idx_count = -1;
        _running_command->press();
      }
      else if (_state == State::Tap_or_NextCommand)
      {
        CommandTapper.tap(_pairs[_idx_count].tap_command);
        _idx_count = -1;
        _state = State::Unexecuted;
        stopListen_BeforeDifferentRootCommandPress();
      }
    }

    //------------------------------------------------------------------+
    // TapDanceDetermineWithMouseMove
    //------------------------------------------------------------------+
    TapDanceDetermineWithMouseMove::TapDanceDetermineWithMouseMove(Pair pairs[], uint8_t len, uint8_t mouse_ids[], uint8_t mouse_ids_len)
        : TimerMixin(), BdrcpEventListener(this), BmmEventListener(),
          _pairs(pairs), _len(len), _mouse_ids(mouse_ids), _mouse_ids_len(mouse_ids_len), _idx_count(-1), _state(State::Unexecuted)
    {
      for (int i = 0; i < len; i++)
      {
        pairs[i].tap_command->setParent(this);
        pairs[i].hold_command->setParent(this);
      }
    }

    void TapDanceDetermineWithMouseMove::onPress(uint8_t n_times)
    {
      if (_state == State::Unexecuted)
      {
        startListen();
      }

      if (_state == State::Unexecuted || _state == State::Tap_or_NextCommand)
      {
        _idx_count++;
        _state = State::Pressed;
        startTimer(HID_ENGINE_TAPPING_TERM_MS);
      }
    }

    uint8_t TapDanceDetermineWithMouseMove::onRelease()
    {
      if (_state == State::FixedToHold)
      {
        _running_command->release();
        _state = State::Unexecuted;
        stopListen();
      }
      else if (_state == State::Pressed)
      {
        if (_idx_count == _len - 1)
        {
          CommandTapper.tap(_pairs[_idx_count].tap_command);
          _idx_count = -1;
          _state = State::Unexecuted;
          stopListen();
        }
        else
        {
          _state = State::Tap_or_NextCommand;
          startTimer(HID_ENGINE_TAPPING_TERM_MS);
        }
      }
      return 1;
    }

    void TapDanceDetermineWithMouseMove::onTimer()
    {
      if (_state == State::Pressed)
      {
        _state = State::FixedToHold;
        _running_command = _pairs[_idx_count].hold_command;
        _idx_count = -1;
        _running_command->press();
      }
      else if (_state == State::Tap_or_NextCommand)
      {
        CommandTapper.tap(_pairs[_idx_count].tap_command);
        _idx_count = -1;
        _state = State::Unexecuted;
        stopListen();
      }
    }

    void TapDanceDetermineWithMouseMove::onBeforeInput()
    {
      if (_state == State::Pressed)
      {
        _state = State::FixedToHold;
        _running_command = _pairs[_idx_count].hold_command;
        _idx_count = -1;
        _running_command->press();
      }
      else if (_state == State::Tap_or_NextCommand)
      {
        CommandTapper.tap(_pairs[_idx_count].tap_command);
        _idx_count = -1;
        _state = State::Unexecuted;
        stopListen();
      }
    }

    void TapDanceDetermineWithMouseMove::onBeforeDifferentRootCommandPress()
    {
      onBeforeInput();
    }

    void TapDanceDetermineWithMouseMove::onBeforeMouseMove(uint8_t mouse_id)
    {
      for (int i = 0; i < _mouse_ids_len; i++)
      {
        if (_mouse_ids[i] == mouse_id)
        {
          onBeforeInput();
        }
      }
    }

    void TapDanceDetermineWithMouseMove::startListen()
    {
      startListen_BeforeDifferentRootCommandPress();
      startListen_BeforeMouseMove();
    }

    void TapDanceDetermineWithMouseMove::stopListen()
    {
      stopListen_BeforeDifferentRootCommandPress();
      stopListen_BeforeMouseMove();
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
        _state = State::Pressed;
        startTimer(_ms);
      }
    }

    uint8_t TapOrHold::onRelease()
    {
      if (_state == State::Pressed)
      {
        CommandTapper.tap(_tap_command);
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
      if (_state == State::Pressed)
      {
        _state = State::FixedToHold;
        _hold_command->press();
      }
    }

    //------------------------------------------------------------------+
    // ConsumerControl
    //------------------------------------------------------------------+
    ConsumerControl::ConsumerControl(ConsumerControlCode usage_code) : _usage_code(usage_code)
    {
    }

    void ConsumerControl::onPress(uint8_t n_times)
    {
      Hid.consumerKeyPress(_usage_code);
    }
    uint8_t ConsumerControl::onRelease()
    {
      Hid.consumerKeyRelease();
      return 1;
    }

    //------------------------------------------------------------------+
    // SystemControl
    //------------------------------------------------------------------+
    SystemControl::SystemControl(SystemControlCode usage_code) : _usage_code(usage_code)
    {
    }

    void SystemControl::onPress(uint8_t n_times)
    {
      Hid.systemControlKeyPress(_usage_code);
    }
    uint8_t SystemControl::onRelease()
    {
      Hid.systemControlKeyRelease();
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
    MouseScroll::MouseScroll(int8_t scroll, int8_t horiz)
        : _scroll(scroll), _horiz(horiz), _max_n_times(127 / std::max(abs(_scroll), abs(_horiz)))
    {
    }

    void MouseScroll::onPress(uint8_t n_times)
    {
      _actual_n_times = std::min(n_times, _max_n_times);
      Hid.mouseScroll(_scroll * _actual_n_times, _horiz * _actual_n_times);
    }

    uint8_t MouseScroll::onRelease()
    {
      return _actual_n_times;
    }

    //------------------------------------------------------------------+
    // MouseClick
    //------------------------------------------------------------------+
    MouseClick::MouseClick(MouseButtons buttons) : _buttons(buttons)
    {
    }

    void MouseClick::onPress(uint8_t n_times)
    {
      Hid.mouseButtonsPress(_buttons);
    }

    uint8_t MouseClick::onRelease()
    {
      Hid.mouseButtonsRelease(_buttons);
      return 1;
    }

    //------------------------------------------------------------------+
    // RadialClick
    //------------------------------------------------------------------+

    void RadialClick::onPress(uint8_t n_times)
    {
      Hid.radialControllerButtonPress();
    }

    uint8_t RadialClick::onRelease()
    {
      Hid.radialControllerButtonRelease();
      return 1;
    }

    //------------------------------------------------------------------+
    // RadialRotate
    //------------------------------------------------------------------+
    RadialRotate::RadialRotate(int16_t deci_degree)
        : _deci_degree(deci_degree), _max_n_times(std::min(3600 / abs(_deci_degree), UINT8_MAX))
    {
    }

    void RadialRotate::onPress(uint8_t n_times)
    {
      _actual_n_times = std::min(n_times, _max_n_times);
      Hid.radialControllerDialRotate(_deci_degree * _actual_n_times);
    }

    uint8_t RadialRotate::onRelease()
    {
      return _actual_n_times;
    }

    //------------------------------------------------------------------+
    // OnceEvery
    //------------------------------------------------------------------+
    OnceEvery::OnceEvery(uint32_t ms, Command *command)
        : _ms(ms), _command(command), _last_press_millis(0), _has_pressed(false)
    {
      _command->setParent(this);
    }

    void OnceEvery::onPress(uint8_t n_times)
    {
      _n_times = n_times;

      uint32_t current_millis = millis();
      if (static_cast<uint32_t>(current_millis - _last_press_millis) >= _ms)
      {
        _command->press();
        _last_press_millis = current_millis;
        _has_pressed = true;
      }
    }

    uint8_t OnceEvery::onRelease()
    {
      if (_has_pressed)
      {
        _command->release();
        _has_pressed = false;
      }

      return _n_times;
    }

    //------------------------------------------------------------------+
    // TapSpacing
    //------------------------------------------------------------------+
    TapSpacing::TapSpacing(uint32_t reference_ms, Command *command)
        : _reference_ms(reference_ms), _command(command), _last_press_millis(0), _has_pressed(false)
    {
      _command->setParent(this);
    }

    void TapSpacing::onPress(uint8_t n_times)
    {
      _n_times = n_times;

      uint32_t ms = _reference_ms / n_times;

      uint32_t current_millis = millis();
      if (static_cast<uint32_t>(current_millis - _last_press_millis) >= ms)
      {
        _command->press();
        _last_press_millis = current_millis;
        _has_pressed = true;
      }
    }

    uint8_t TapSpacing::onRelease()
    {
      if (_has_pressed)
      {
        _command->release();
        _has_pressed = false;
      }

      return _n_times;
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
      _running_command->press(n_times);
    }

    uint8_t If::onRelease()
    {
      return _running_command->release();
    }

    //------------------------------------------------------------------+
    // Multi
    //------------------------------------------------------------------+
    Multi::Multi(Command *commands[], uint8_t len)
        : _commands(commands), _len(len)
    {
      for (size_t i = 0; i < len; i++)
      {
        _commands[i]->setParent(this);
      }
    }

    void Multi::onPress(uint8_t n_times)
    {
      for (size_t i = 0; i < _len; i++)
      {
        _commands[i]->press();
      }
    }

    uint8_t Multi::onRelease()
    {
      for (size_t i = 0; i < _len; i++)
      {
        _commands[i]->release();
      }
      return 1;
    }

    //------------------------------------------------------------------+
    // NoOperation
    //------------------------------------------------------------------+
    void NoOperation::onPress(uint8_t n_times)
    {
      _n_times = n_times;
    }

    uint8_t NoOperation::onRelease()
    {
      return _n_times;
    }

  } // namespace Internal

} // namespace hidpg
