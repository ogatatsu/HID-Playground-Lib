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

#include "CommandImpl.h"
#include "ArduinoMacro.h"
#include "CommandTapper.h"
#include "HidCore.h"
#include "utility.h"

namespace hidpg::Internal
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
  // Layering
  //------------------------------------------------------------------+
  Layering::Layering(LayerClass &layer, const etl::span<CommandPtr> commands) : _layer(layer), _commands(commands)
  {
    for (auto command : _commands)
    {
      if (command != nullptr)
      {
        command->setParent(this);
      }
    }
  }

  void Layering::onPress(uint8_t n_times)
  {
    // 現在のレイヤーの状態を取得
    layer_bitmap_t layer_state = _layer.getState();

    _running_command = nullptr;

    // layerを上から舐めていってonのlayerを探す
    int i = _commands.size() - 1;
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
    // 実行する
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
  // ToggleLayer
  //------------------------------------------------------------------+
  ToggleLayer::ToggleLayer(LayerClass &layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void ToggleLayer::onPress(uint8_t n_times)
  {
    _layer.toggle(_layer_number);
  }

  //------------------------------------------------------------------+
  // SwitchLayer
  //------------------------------------------------------------------+
  SwitchLayer::SwitchLayer(LayerClass &layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void SwitchLayer::onPress(uint8_t n_times)
  {
    _layer.on(_layer_number);
  }

  uint8_t SwitchLayer::onRelease()
  {
    _layer.off(_layer_number);
    return 1;
  }

  //------------------------------------------------------------------+
  // UpDefaultLayer
  //------------------------------------------------------------------+
  UpDefaultLayer::UpDefaultLayer(LayerClass &layer, uint8_t i) : _layer(layer), _i(i)
  {
  }

  void UpDefaultLayer::onPress(uint8_t n_times)
  {
    _layer.addToDefaultLayer(_i);
  }

  uint8_t UpDefaultLayer::onRelease()
  {
    _layer.addToDefaultLayer(-_i);
    return 1;
  }

  //------------------------------------------------------------------+
  // Tap
  //------------------------------------------------------------------+
  Tap::Tap(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms)
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
  TapWhenReleased::TapWhenReleased(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms)
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
  TapDance::TapDance(etl::span<Pair> pairs,
                     etl::span<MouseId> mouse_ids,
                     uint16_t move_threshold,
                     HoldTapBehavior behavior)
      : TimerMixin(),
        BeforeOtherCommandPressEventListener(this),
        BeforeMouseMoveEventListener(),
        CommandHook(),
        _pairs(pairs),
        _mouse_ids(mouse_ids),
        _move_threshold(move_threshold),
        _behavior(behavior),
        _delta_x_sum(0),
        _delta_y_sum(0),
        _idx_count(-1),
        _state(State::Unexecuted)
  {
    for (auto &pair : _pairs)
    {
      pair.hold_command->setParent(this);
      if (pair.tap_command != nullptr)
      {
        pair.tap_command->setParent(this);
      }
    }
  }

  void TapDance::performTap()
  {
    Command &cmd = (_pairs[_idx_count].tap_command != nullptr)
                       ? *(_pairs[_idx_count].tap_command)
                       : *(_pairs[_idx_count].hold_command.get());
    cmd.press();
    cmd.release();

    _idx_count = -1;
    stopTimer();
    stopListenBeforeOtherCommandPress();
    stopListenBeforeRotateEncoder();
    stopListenBeforeMouseMove();
  }

  void TapDance::performHoldPress()
  {
    _running_command = _pairs[_idx_count].hold_command;
    _running_command->press();

    _idx_count = -1;
    stopTimer();
    stopListenBeforeOtherCommandPress();
    stopListenBeforeRotateEncoder();
    stopListenBeforeMouseMove();
  }

  void TapDance::performHoldRelease()
  {
    _running_command->release();
  }

  bool TapDance::checkMoveThreshold(BeforeMouseMoveArgs &args)
  {
    for (auto &id : _mouse_ids)
    {
      if (id == args.mouse_id)
      {
        _delta_x_sum = etl::clamp(_delta_x_sum + args.delta_x, INT16_MIN, INT16_MAX);
        _delta_y_sum = etl::clamp(_delta_y_sum + args.delta_y, INT16_MIN, INT16_MAX);
        if (abs(_delta_x_sum) >= _move_threshold || abs(_delta_y_sum) >= _move_threshold)
        {
          return true;
        }
      }
    }
    return false;
  }

  void TapDance::processTapDance(Action action, ArgsType args)
  {
    if (_behavior == HoldTapBehavior::HoldPreferred)
    {
      processHoldPreferredBehavior(action, args);
    }
    else if (_behavior == HoldTapBehavior::Balanced)
    {
      processBalancedBehavior(action, args);
    }
  }

  void TapDance::processHoldPreferredBehavior(Action action, ArgsType &args)
  {
    switch (Context(action, _state))
    {
    case Context(Action::Press, State::Unexecuted):
    {
      _state = State::Pressed;
      startListenBeforeOtherCommandPress();
      if (_mouse_ids.empty() == false)
      {
        _delta_x_sum = 0;
        _delta_y_sum = 0;
        startListenBeforeMouseMove();
      }
      startListenBeforeRotateEncoder();
      _idx_count++;
      startTimer(HID_ENGINE_TAPPING_TERM_MS);
    }
    break;

    case Context(Action::Press, State::TapOrNextCommand):
    {
      _state = State::Pressed;
      _idx_count++;
      startTimer(HID_ENGINE_TAPPING_TERM_MS);
    }
    break;

    case Context(Action::Release, State::Pressed):
    {
      if (_idx_count == _pairs.size() - 1)
      {
        _state = State::Unexecuted;
        performTap();
      }
      else
      {
        _state = State::TapOrNextCommand;
        startTimer(HID_ENGINE_TAPPING_TERM_MS);
      }
    }
    break;

    case Context(Action::Release, State::DecidedToHold):
    {
      _state = State::Unexecuted;
      performHoldRelease();
    }
    break;

    case Context(Action::Timer, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::Timer, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::BeforeOtherCommandPress, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::BeforeOtherCommandPress, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::BeforeMouseMove, State::Pressed):
    {
      if (checkMoveThreshold(etl::get<BeforeMouseMoveArgs>(args)))
      {
        _state = State::DecidedToHold;
        performHoldPress();
      }
    }
    break;

    case Context(Action::BeforeMouseMove, State::TapOrNextCommand):
    {
      if (checkMoveThreshold(etl::get<BeforeMouseMoveArgs>(args)))
      {
        _state = State::Unexecuted;
        performTap();
      }
    }
    break;

    case Context(Action::BeforeRotateEncoder, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::BeforeRotateEncoder, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    default:
      break;
    }
  }

  void TapDance::processBalancedBehavior(Action action, ArgsType &args)
  {
    switch (Context(action, _state))
    {
    case Context(Action::Press, State::Unexecuted):
    {
      _state = State::Pressed;
      startListenBeforeOtherCommandPress();
      if (_mouse_ids.empty() == false)
      {
        _delta_x_sum = 0;
        _delta_y_sum = 0;
        startListenBeforeMouseMove();
      }
      startListenBeforeRotateEncoder();
      _idx_count++;
      startTimer(HID_ENGINE_TAPPING_TERM_MS);
    }
    break;

    case Context(Action::Press, State::TapOrNextCommand):
    {
      _state = State::Pressed;
      _idx_count++;
      startTimer(HID_ENGINE_TAPPING_TERM_MS);
    }
    break;

    case Context(Action::Release, State::Pressed):
    {
      if (_idx_count == _pairs.size() - 1)
      {
        _state = State::Unexecuted;
        performTap();
      }
      else
      {
        _state = State::TapOrNextCommand;
        startTimer(HID_ENGINE_TAPPING_TERM_MS);
      }
    }
    break;

    case Context(Action::Release, State::DecidedToHold):
    {
      _state = State::Unexecuted;
      performHoldRelease();
    }
    break;

    case Context(Action::Release, State::Hook):
    {
      _state = State::Unexecuted;
      performTap();
      stopHook();
      _hooked_command->press();
    }
    break;

    case Context(Action::Timer, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::Timer, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::Timer, State::Hook):
    {
      _state = State::DecidedToHold;
      performHoldPress();
      stopHook();
      _hooked_command->press();
    }
    break;

    case Context(Action::BeforeOtherCommandPress, State::Pressed):
    {
      BeforeOtherCommandPressArgs &_args = etl::get<BeforeOtherCommandPressArgs>(args);
      if (startHook(_args.command))
      {
        _state = State::Hook;
        _hooked_command = &_args.command;
      }
    }
    break;

    case Context(Action::BeforeOtherCommandPress, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::BeforeOtherCommandPress, State::Hook):
    {
      _state = State::DecidedToHold;
      performHoldPress();
      stopHook();
      _hooked_command->press();
    }
    break;

    case Context(Action::BeforeMouseMove, State::Pressed):
    {
      if (checkMoveThreshold(etl::get<BeforeMouseMoveArgs>(args)))
      {
        _state = State::DecidedToHold;
        performHoldPress();
      }
    }
    break;

    case Context(Action::BeforeMouseMove, State::TapOrNextCommand):
    {
      if (checkMoveThreshold(etl::get<BeforeMouseMoveArgs>(args)))
      {
        _state = State::Unexecuted;
        performTap();
      }
    }
    break;

    case Context(Action::BeforeRotateEncoder, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::BeforeRotateEncoder, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::HookRelease, State::Hook):
    {
      _state = State::DecidedToHold;
      performHoldPress();
      stopHook();
      _hooked_command->press();
      _hooked_command->release();
    }
    break;

    default:
      break;
    }
  }

  void TapDance::onPress(uint8_t n_times)
  {
    processTapDance(Action::Press, nullptr);
  }

  uint8_t TapDance::onRelease()
  {
    processTapDance(Action::Release, nullptr);
    return 1;
  }

  void TapDance::onTimer()
  {
    processTapDance(Action::Timer, nullptr);
  }

  void TapDance::onBeforeOtherCommandPress(Command &command)
  {
    BeforeOtherCommandPressArgs args{.command = command};
    processTapDance(Action::BeforeOtherCommandPress, args);
  }

  void TapDance::onBeforeMouseMove(MouseId mouse_id, int16_t delta_x, int16_t delta_y)
  {
    BeforeMouseMoveArgs args{.mouse_id = mouse_id, .delta_x = delta_x, .delta_y = delta_y};
    processTapDance(Action::BeforeMouseMove, args);
  }

  void TapDance::onBeforeRotateEncoder(EncoderId encoder_id, int16_t step)
  {
    BeforeRotateEncoderArgs args{.encoder_id = encoder_id, .step = step};
    processTapDance(Action::BeforeRotateEncoder, args);
  }

  void TapDance::onHookPress()
  {
    processTapDance(Action::HookPress, nullptr);
  }

  void TapDance::onHookRelease()
  {
    processTapDance(Action::HookRelease, nullptr);
  }

  //------------------------------------------------------------------+
  // TapOrHold
  //------------------------------------------------------------------+
  TapOrHold::TapOrHold(NotNullCommandPtr tap_command, unsigned int ms, NotNullCommandPtr hold_command)
      : TimerMixin(), _tap_command(tap_command), _hold_command(hold_command), _ms(ms), _state(State::Unexecuted)
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
  MouseMove::MouseMove(int16_t x, int16_t y)
      : _x(x), _y(y), _max_n_times(std::min(32767 / std::max(abs(_x), abs(_y)), UINT8_MAX))
  {
  }

  void MouseMove::onPress(uint8_t n_times)
  {
    _actual_n_times = std::min(n_times, _max_n_times);
    Hid.mouseMove(_x * _actual_n_times, _y * _actual_n_times);
  }

  uint8_t MouseMove::onRelease()
  {
    return _actual_n_times;
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
  OnceEvery::OnceEvery(NotNullCommandPtr command, uint32_t ms)
      : _command(command), _ms(ms), _last_press_millis(0), _has_pressed(false)
  {
    _command->setParent(this);
  }

  void OnceEvery::onPress(uint8_t n_times)
  {
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

    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // NTimesEvery
  //------------------------------------------------------------------+
  NTimesEvery::NTimesEvery(NotNullCommandPtr command, uint32_t ms)
      : _command(command), _ms(ms), _last_press_millis(0), _has_pressed(false)
  {
    _command->setParent(this);
  }

  void NTimesEvery::onPress(uint8_t n_times)
  {
    uint32_t ms = _ms / n_times;

    uint32_t current_millis = millis();
    if (static_cast<uint32_t>(current_millis - _last_press_millis) >= ms)
    {
      _command->press();
      _last_press_millis = current_millis;
      _has_pressed = true;
    }
  }

  uint8_t NTimesEvery::onRelease()
  {
    if (_has_pressed)
    {
      _command->release();
      _has_pressed = false;
    }

    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // If
  //------------------------------------------------------------------+
  If::If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command)
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
  Multi::Multi(etl::span<NotNullCommandPtr> commands)
      : _commands(commands)
  {
    for (auto &command : _commands)
    {
      command->setParent(this);
    }
  }

  void Multi::onPress(uint8_t n_times)
  {
    for (auto &command : _commands)
    {
      command->press();
    }
  }

  uint8_t Multi::onRelease()
  {
    for (auto &command : _commands)
    {
      command->release();
    }
    return 1;
  }

  //------------------------------------------------------------------+
  // Toggle
  //------------------------------------------------------------------+
  Toggle::Toggle(NotNullCommandPtr command)
      : _command(command), _is_pressed(false)
  {
    _command->setParent(this);
  }

  void Toggle::onPress(uint8_t n_times)
  {
    if (_is_pressed == false)
    {
      _command->press();
    }
    else
    {
      _command->release();
    }

    _is_pressed = !_is_pressed;
  }

  //------------------------------------------------------------------+
  // Repeat
  //------------------------------------------------------------------+
  Repeat::Repeat(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms)
      : TimerMixin(), _command(command), _delay_ms(delay_ms), _interval_ms(interval_ms)
  {
  }

  void Repeat::onPress(uint8_t n_times)
  {
    CommandTapper.tap(_command);
    startTimer(_delay_ms);
  }

  uint8_t Repeat::onRelease()
  {
    stopTimer();
    return 1;
  }

  void Repeat::onTimer()
  {
    CommandTapper.tap(_command);
    startTimer(_interval_ms);
  }

  //------------------------------------------------------------------+
  // Cycle
  //------------------------------------------------------------------+
  Cycle::Cycle(etl::span<NotNullCommandPtr> commands)
      : _commands(commands), _idx(0)
  {
    for (auto &command : _commands)
    {
      command->setParent(this);
    }
  }

  void Cycle::onPress(uint8_t n_times)
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->press();
  }

  uint8_t Cycle::onRelease()
  {
    if (_commands.size() == 0)
    {
      return UINT8_MAX;
    }

    _commands[_idx]->release();
    _idx = (_idx + 1) % _commands.size();

    return 1;
  }

  //------------------------------------------------------------------+
  // CyclePhaseShift
  //------------------------------------------------------------------+
  CyclePhaseShift::CyclePhaseShift(etl::span<NotNullCommandPtr> commands)
      : _commands(commands), _idx(_commands.size() - 1)
  {
    for (auto &command : _commands)
    {
      command->setParent(this);
    }
  }

  void CyclePhaseShift::onPress(uint8_t n_times)
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->release();
    _idx = (_idx + 1) % _commands.size();
  }

  uint8_t CyclePhaseShift::onRelease()
  {
    if (_commands.size() == 0)
    {
      return UINT8_MAX;
    }

    _commands[_idx]->press();

    return 1;
  }

  //------------------------------------------------------------------+
  // NoOperation
  //------------------------------------------------------------------+
  uint8_t NoOperation::onRelease()
  {
    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // GestureCommand
  //------------------------------------------------------------------+
  GestureCommand::GestureCommand(GestureId gesture_id) : _gesture_id(gesture_id.value)
  {
  }

  void GestureCommand::onPress(uint8_t n_times)
  {
    HidEngine.startGesture(_gesture_id);
  }

  uint8_t GestureCommand::onRelease()
  {
    HidEngine.stopGesture(_gesture_id);
    return 1;
  }

  //------------------------------------------------------------------+
  // GestureOr
  //------------------------------------------------------------------+
  GestureOr::GestureOr(GestureId gesture_id, NotNullCommandPtr command)
      : BeforeOtherCommandPressEventListener(this),
        BeforeGestureEventListener(),
        _gesture_id(gesture_id.value),
        _command(command),
        _state(State::Unexecuted)
  {
    _command->setParent(this);
  }

  void GestureOr::onPress(uint8_t n_times)
  {
    _state = State::Pressed;
    HidEngine.startGesture(_gesture_id);
    startListen();
  }

  uint8_t GestureOr::onRelease()
  {
    if (_state == State::Pressed)
    {
      _state = State::Unexecuted;
      HidEngine.stopGesture(_gesture_id);
      stopListen();
      CommandTapper.tap(_command);
    }
    else if (_state == State::OtherCommandPressed)
    {
      _state = State::Unexecuted;
      _command->release();
    }
    else if (_state == State::Gestured)
    {
      _state = State::Unexecuted;
      HidEngine.stopGesture(_gesture_id);
    }

    return 1;
  }

  void GestureOr::onBeforeOtherCommandPress(Command &command)
  {
    if (_state == State::Pressed)
    {
      _state = State::OtherCommandPressed;
      HidEngine.stopGesture(_gesture_id);
      stopListen();
      _command->press();
    }
  }

  void GestureOr::onBeforeGesture(GestureId gesture_id, MouseId mouse_id)
  {
    if (_state == State::Pressed)
    {
      _state = State::Gestured;
      stopListen();
    }
  }

  void GestureOr::startListen()
  {
    startListenBeforeOtherCommandPress();
    startListenBeforeGesture();
  }

  void GestureOr::stopListen()
  {
    stopListenBeforeOtherCommandPress();
    stopListenBeforeGesture();
  }

  //------------------------------------------------------------------+
  // GestureOrNK
  //------------------------------------------------------------------+
  GestureOrNK::GestureOrNK(GestureId gesture_id, KeyCode key_code)
      : BeforeOtherCommandPressEventListener(this),
        BeforeGestureEventListener(),
        _gesture_id(gesture_id.value),
        _nk_command(key_code),
        _state(State::Unexecuted)
  {
    _nk_command.setParent(this);
  }

  void GestureOrNK::onPress(uint8_t n_times)
  {
    if (Hid.isModifiersSet())
    {
      _state = State::PressedWithModifiers;
      _nk_command.press();
    }
    else
    {
      _state = State::Pressed;
      HidEngine.startGesture(_gesture_id);
      startListen();
    }
  }

  uint8_t GestureOrNK::onRelease()
  {
    if (_state == State::Pressed)
    {
      _state = State::Unexecuted;
      HidEngine.stopGesture(_gesture_id);
      stopListen();
      CommandTapper.tap(&_nk_command);
    }
    else if (_state == State::OtherCommandPressed || _state == State::PressedWithModifiers)
    {
      _state = State::Unexecuted;
      _nk_command.release();
    }
    else if (_state == State::Gestured)
    {
      _state = State::Unexecuted;
      HidEngine.stopGesture(_gesture_id);
    }

    return 1;
  }

  void GestureOrNK::onBeforeOtherCommandPress(Command &command)
  {
    if (_state == State::Pressed)
    {
      _state = State::OtherCommandPressed;
      HidEngine.stopGesture(_gesture_id);
      stopListen();
      _nk_command.press();
    }
  }

  void GestureOrNK::onBeforeGesture(GestureId gesture_id, MouseId mouse_id)
  {
    if (_state == State::Pressed)
    {
      _state = State::Gestured;
      stopListen();
    }
  }

  void GestureOrNK::startListen()
  {
    startListenBeforeOtherCommandPress();
    startListenBeforeGesture();
  }

  void GestureOrNK::stopListen()
  {
    stopListenBeforeOtherCommandPress();
    stopListenBeforeGesture();
  }

  //------------------------------------------------------------------+
  // EncoderShift
  //------------------------------------------------------------------+
  EncoderShift::EncoderShift(EncoderShiftId encoder_shift_id) : _encoder_shift_id_link(encoder_shift_id.value)
  {
  }

  void EncoderShift::onPress(uint8_t n_times)
  {
    HidEngine.startEncoderShift(_encoder_shift_id_link);
  }

  uint8_t EncoderShift::onRelease()
  {
    HidEngine.stopEncoderShift(_encoder_shift_id_link);
    return 1;
  }

} // namespace hidpg::Internal
