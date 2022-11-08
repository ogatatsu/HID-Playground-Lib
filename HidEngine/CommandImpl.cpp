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

  void NormalKey::onPress()
  {
    Hid.setKey(_key_code);
    Hid.sendKeyReport();
  }

  void NormalKey::onRelease()
  {
    Hid.unsetKey(_key_code);
    Hid.sendKeyReport();
  }

  //------------------------------------------------------------------+
  // ModifierKey
  //------------------------------------------------------------------+
  ModifierKey::ModifierKey(Modifiers modifiers) : _modifiers(modifiers)
  {
  }

  void ModifierKey::onPress()
  {
    Hid.setModifiers(_modifiers);
    Hid.sendKeyReport();
  }

  void ModifierKey::onRelease()
  {
    Hid.unsetModifiers(_modifiers);
    Hid.sendKeyReport();
  }

  //------------------------------------------------------------------+
  // CombinationKey
  //------------------------------------------------------------------+
  CombinationKey::CombinationKey(Modifiers modifiers, KeyCode key_code) : _modifiers(modifiers), _key_code(key_code)
  {
  }

  void CombinationKey::onPress()
  {
    Hid.setKey(_key_code);
    Hid.setModifiers(_modifiers);
    Hid.sendKeyReport();
  }

  void CombinationKey::onRelease()
  {
    Hid.unsetKey(_key_code);
    Hid.unsetModifiers(_modifiers);
    Hid.sendKeyReport();
  }

  //------------------------------------------------------------------+
  // Layering
  //------------------------------------------------------------------+
  Layering::Layering(LayerClass &layer, const etl::span<CommandPtr> commands) : _layer(layer), _commands(commands)
  {
  }

  void Layering::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);

    for (auto &command : _commands)
    {
      if (command != nullptr)
      {
        command->setKeyId(key_id);
      }
    }
  }

  void Layering::onPress()
  {
    _running_command = getCurrentCommand();
    if (_running_command != nullptr)
    {
      _running_command->press();
    }
  }

  void Layering::onRelease()
  {
    if (_running_command != nullptr)
    {
      _running_command->release();
    }
  }

  uint8_t Layering::onTap(uint8_t n_times)
  {
    CommandPtr cmd = getCurrentCommand();
    return cmd->tap(n_times);
  }

  CommandPtr Layering::getCurrentCommand()
  {
    // 現在のレイヤーの状態を取得
    layer_bitmap_t layer_state = _layer.getState();

    CommandPtr result = nullptr;

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
      result = _commands[i];
      break;
    }

    return result;
  }

  //------------------------------------------------------------------+
  // ToggleLayer
  //------------------------------------------------------------------+
  ToggleLayer::ToggleLayer(LayerClass &layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void ToggleLayer::onPress()
  {
    _layer.toggle(_layer_number);
  }

  //------------------------------------------------------------------+
  // SwitchLayer
  //------------------------------------------------------------------+
  SwitchLayer::SwitchLayer(LayerClass &layer, uint8_t layer_number) : _layer(layer), _layer_number(layer_number)
  {
  }

  void SwitchLayer::onPress()
  {
    _layer.on(_layer_number);
  }

  void SwitchLayer::onRelease()
  {
    _layer.off(_layer_number);
  }

  //------------------------------------------------------------------+
  // UpDefaultLayer
  //------------------------------------------------------------------+
  UpDefaultLayer::UpDefaultLayer(LayerClass &layer, uint8_t i) : _layer(layer), _i(i)
  {
  }

  void UpDefaultLayer::onPress()
  {
    _layer.addToDefaultLayer(_i);
  }

  void UpDefaultLayer::onRelease()
  {
    _layer.addToDefaultLayer(-_i);
  }

  //------------------------------------------------------------------+
  // Tap
  //------------------------------------------------------------------+
  Tap::Tap(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms)
      : _command(command), _n_times(n_times), _tap_speed_ms(tap_speed_ms)
  {
  }

  void Tap::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void Tap::onPress()
  {
    CommandTapper.tap(_command, _n_times, _tap_speed_ms);
  }

  //------------------------------------------------------------------+
  // TapWhenReleased
  //------------------------------------------------------------------+
  TapWhenReleased::TapWhenReleased(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms)
      : _command(command), _n_times(n_times), _tap_speed_ms(tap_speed_ms)
  {
  }

  void TapWhenReleased::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void TapWhenReleased::onRelease()
  {
    CommandTapper.tap(_command, _n_times, _tap_speed_ms);
  }

  //------------------------------------------------------------------+
  // TapDance
  //------------------------------------------------------------------+
  TapDance::TapDance(etl::span<Pair> pairs,
                     etl::span<PointingDeviceId> pointing_device_ids,
                     uint16_t move_threshold,
                     uint32_t tapping_term_ms)
      : TimerMixin(),
        BeforeOtherKeyPressEventListener(this),
        BeforeMovePointerEventListener(),
        _pairs(pairs),
        _pointing_device_ids(pointing_device_ids),
        _move_threshold(move_threshold),
        _tapping_term_ms(tapping_term_ms),
        _delta_x_sum(0),
        _delta_y_sum(0),
        _idx_count(-1),
        _state(State::Unexecuted)
  {
  }

  void TapDance::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);

    for (auto &pair : _pairs)
    {
      pair.hold_command->setKeyId(key_id);
      if (pair.tap_command != nullptr)
      {
        pair.tap_command->setKeyId(key_id);
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
    stopListenBeforeOtherKeyPress();
    stopListenBeforeRotateEncoder();
    stopListenBeforeMovePointer();
  }

  void TapDance::performHoldPress()
  {
    _running_command = _pairs[_idx_count].hold_command;
    _running_command->press();

    _idx_count = -1;
    stopTimer();
    stopListenBeforeOtherKeyPress();
    stopListenBeforeRotateEncoder();
    stopListenBeforeMovePointer();
  }

  void TapDance::performHoldRelease()
  {
    _running_command->release();
  }

  bool TapDance::checkMoveThreshold(BeforeMovePointerArgs &args)
  {
    for (auto &id : _pointing_device_ids)
    {
      if (id == args.pointing_device_id)
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
    switch (Context(action, _state))
    {
    case Context(Action::Press, State::Unexecuted):
    {
      _state = State::Pressed;
      startListenBeforeOtherKeyPress();
      if (_pointing_device_ids.empty() == false)
      {
        _delta_x_sum = 0;
        _delta_y_sum = 0;
        startListenBeforeMovePointer();
      }
      startListenBeforeRotateEncoder();
      _idx_count++;
      startTimer(_tapping_term_ms);
    }
    break;

    case Context(Action::Press, State::TapOrNextCommand):
    {
      _state = State::Pressed;
      _idx_count++;
      startTimer(_tapping_term_ms);
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
        startTimer(_tapping_term_ms);
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

    case Context(Action::BeforeOtherKeyPress, State::Pressed):
    {
      _state = State::DecidedToHold;
      performHoldPress();
    }
    break;

    case Context(Action::BeforeOtherKeyPress, State::TapOrNextCommand):
    {
      _state = State::Unexecuted;
      performTap();
    }
    break;

    case Context(Action::BeforeMouseMove, State::Pressed):
    {
      if (checkMoveThreshold(etl::get<BeforeMovePointerArgs>(args)))
      {
        _state = State::DecidedToHold;
        performHoldPress();
      }
    }
    break;

    case Context(Action::BeforeMouseMove, State::TapOrNextCommand):
    {
      if (checkMoveThreshold(etl::get<BeforeMovePointerArgs>(args)))
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

  void TapDance::onPress()
  {
    processTapDance(Action::Press, nullptr);
  }

  void TapDance::onRelease()
  {
    processTapDance(Action::Release, nullptr);
  }

  void TapDance::onTimer()
  {
    processTapDance(Action::Timer, nullptr);
  }

  void TapDance::onBeforeOtherKeyPress(uint8_t key_id)
  {
    BeforeOtherKeyPressArgs args{.key_id = key_id};
    processTapDance(Action::BeforeOtherKeyPress, args);
  }

  void TapDance::onBeforeMovePointer(PointingDeviceId pointing_device_id, int16_t delta_x, int16_t delta_y)
  {
    BeforeMovePointerArgs args{.pointing_device_id = pointing_device_id, .delta_x = delta_x, .delta_y = delta_y};
    processTapDance(Action::BeforeMouseMove, args);
  }

  void TapDance::onBeforeRotateEncoder(EncoderId encoder_id, int16_t step)
  {
    BeforeRotateEncoderArgs args{.encoder_id = encoder_id, .step = step};
    processTapDance(Action::BeforeRotateEncoder, args);
  }

  //------------------------------------------------------------------+
  // ConsumerControl
  //------------------------------------------------------------------+
  ConsumerControl::ConsumerControl(ConsumerControlCode usage_code) : _usage_code(usage_code)
  {
  }

  void ConsumerControl::onPress()
  {
    Hid.consumerKeyPress(_usage_code);
  }

  void ConsumerControl::onRelease()
  {
    Hid.consumerKeyRelease();
  }

  //------------------------------------------------------------------+
  // SystemControl
  //------------------------------------------------------------------+
  SystemControl::SystemControl(SystemControlCode usage_code) : _usage_code(usage_code)
  {
  }

  void SystemControl::onPress()
  {
    Hid.systemControlKeyPress(_usage_code);
  }

  void SystemControl::onRelease()
  {
    Hid.systemControlKeyRelease();
  }

  //------------------------------------------------------------------+
  // MouseMove
  //------------------------------------------------------------------+
  MouseMove::MouseMove(int16_t x, int16_t y)
      : _x(x), _y(y), _max_n_times(std::min(32767 / std::max(abs(_x), abs(_y)), UINT8_MAX))
  {
  }

  void MouseMove::onPress()
  {
    Hid.mouseMove(_x, _y);
  }

  uint8_t MouseMove::onTap(uint8_t n_times)
  {
    uint8_t actual_n_times = std::min(n_times, _max_n_times);
    Hid.mouseMove(_x * actual_n_times, _y * actual_n_times);
    return actual_n_times;
  }

  //------------------------------------------------------------------+
  // MouseScroll
  //------------------------------------------------------------------+
  MouseScroll::MouseScroll(int8_t scroll, int8_t horiz)
      : _scroll(scroll), _horiz(horiz), _max_n_times(127 / std::max(abs(_scroll), abs(_horiz)))
  {
  }

  void MouseScroll::onPress()
  {
    Hid.mouseScroll(_scroll, _horiz);
  }

  uint8_t MouseScroll::onTap(uint8_t n_times)
  {
    uint8_t actual_n_times = std::min(n_times, _max_n_times);
    Hid.mouseScroll(_scroll * actual_n_times, _horiz * actual_n_times);
    return actual_n_times;
  }

  //------------------------------------------------------------------+
  // MouseClick
  //------------------------------------------------------------------+
  MouseClick::MouseClick(MouseButtons buttons) : _buttons(buttons)
  {
  }

  void MouseClick::onPress()
  {
    Hid.mouseButtonsPress(_buttons);
  }

  void MouseClick::onRelease()
  {
    Hid.mouseButtonsRelease(_buttons);
  }

  //------------------------------------------------------------------+
  // RadialClick
  //------------------------------------------------------------------+

  void RadialClick::onPress()
  {
    Hid.radialControllerButtonPress();
  }

  void RadialClick::onRelease()
  {
    Hid.radialControllerButtonRelease();
  }

  //------------------------------------------------------------------+
  // RadialRotate
  //------------------------------------------------------------------+
  RadialRotate::RadialRotate(int16_t deci_degree)
      : _deci_degree(deci_degree), _max_n_times(std::min(3600 / abs(_deci_degree), UINT8_MAX))
  {
  }

  void RadialRotate::onPress()
  {
    Hid.radialControllerDialRotate(_deci_degree);
  }

  uint8_t RadialRotate::onTap(uint8_t n_times)
  {
    uint8_t actual_n_times = std::min(n_times, _max_n_times);
    Hid.radialControllerDialRotate(_deci_degree * actual_n_times);
    return actual_n_times;
  }

  //------------------------------------------------------------------+
  // OnceEvery
  //------------------------------------------------------------------+
  OnceEvery::OnceEvery(NotNullCommandPtr command, uint32_t ms)
      : _command(command), _ms(ms), _last_press_millis(0), _has_pressed(false)
  {
  }

  void OnceEvery::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void OnceEvery::onPress()
  {
    uint32_t current_millis = millis();
    if (static_cast<uint32_t>(current_millis - _last_press_millis) >= _ms)
    {
      _command->press();
      _last_press_millis = current_millis;
      _has_pressed = true;
    }
  }

  void OnceEvery::onRelease()
  {
    if (_has_pressed)
    {
      _command->release();
      _has_pressed = false;
    }
  }

  uint8_t OnceEvery::onTap(uint8_t n_times)
  {
    press();
    release();
    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // NTimesEvery
  //------------------------------------------------------------------+
  NTimesEvery::NTimesEvery(NotNullCommandPtr command, uint32_t ms)
      : _command(command), _ms(ms), _last_press_millis(0), _has_pressed(false)
  {
  }

  void NTimesEvery::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void NTimesEvery::onPress()
  {
    uint32_t ms = _n_times.has_value() ? _ms / _n_times.value() : _ms;

    uint32_t current_millis = millis();
    if (static_cast<uint32_t>(current_millis - _last_press_millis) >= ms)
    {
      _command->press();
      _last_press_millis = current_millis;
      _has_pressed = true;
    }
  }

  void NTimesEvery::onRelease()
  {
    if (_has_pressed)
    {
      _command->release();
      _has_pressed = false;
    }
  }

  uint8_t NTimesEvery::onTap(uint8_t n_times)
  {
    _n_times = n_times;
    press();
    release();
    _n_times = etl::nullopt;
    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // If
  //------------------------------------------------------------------+
  If::If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command)
      : _func(func), _true_command(true_command), _false_command(false_command)
  {
  }

  void If::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _true_command->setKeyId(key_id);
    _false_command->setKeyId(key_id);
  }

  void If::onPress()
  {
    _running_command = _func() ? _true_command : _false_command;
    _running_command->press();
  }

  void If::onRelease()
  {
    _running_command->release();
  }

  uint8_t If::onTap(uint8_t n_times)
  {
    auto cmd = _func() ? _true_command : _false_command;
    return cmd->tap(n_times);
  }

  //------------------------------------------------------------------+
  // Multi
  //------------------------------------------------------------------+
  Multi::Multi(etl::span<NotNullCommandPtr> commands)
      : _commands(commands)
  {
  }

  void Multi::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);

    for (auto &command : _commands)
    {
      command->setKeyId(key_id);
    }
  }

  void Multi::onPress()
  {
    for (auto &command : _commands)
    {
      command->press();
    }
  }

  void Multi::onRelease()
  {
    for (auto &command : _commands)
    {
      command->release();
    }
  }

  //------------------------------------------------------------------+
  // Toggle
  //------------------------------------------------------------------+
  Toggle::Toggle(NotNullCommandPtr command)
      : _command(command), _is_pressed(false)
  {
  }

  void Toggle::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void Toggle::onPress()
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

  void Repeat::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);
    _command->setKeyId(key_id);
  }

  void Repeat::onPress()
  {
    CommandTapper.tap(_command);
    startTimer(_delay_ms);
  }

  void Repeat::onRelease()
  {
    stopTimer();
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
  }

  void Cycle::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);

    for (auto &command : _commands)
    {
      command->setKeyId(key_id);
    }
  }

  void Cycle::onPress()
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->press();
  }

  void Cycle::onRelease()
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->release();
    _idx = (_idx + 1) % _commands.size();
  }

  //------------------------------------------------------------------+
  // CyclePhaseShift
  //------------------------------------------------------------------+
  CyclePhaseShift::CyclePhaseShift(etl::span<NotNullCommandPtr> commands)
      : _commands(commands), _idx(_commands.size() - 1)
  {
  }

  void CyclePhaseShift::setKeyId(uint8_t key_id)
  {
    Command::setKeyId(key_id);

    for (auto &command : _commands)
    {
      command->setKeyId(key_id);
    }
  }

  void CyclePhaseShift::onPress()
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->release();
    _idx = (_idx + 1) % _commands.size();
  }

  void CyclePhaseShift::onRelease()
  {
    if (_commands.size() == 0)
    {
      return;
    }

    _commands[_idx]->press();
  }

  //------------------------------------------------------------------+
  // NoOperation
  //------------------------------------------------------------------+
  uint8_t NoOperation::onTap(uint8_t n_times)
  {
    return UINT8_MAX;
  }

  //------------------------------------------------------------------+
  // Shift
  //------------------------------------------------------------------+
  Shift::Shift(etl::span<ShiftIdLink> shift_id_links) : _shift_id_links(shift_id_links)
  {
  }

  void Shift::onPress()
  {
    for (auto &shift_id_link : _shift_id_links)
    {
      if (auto key_shift_id_link = etl::get_if<KeyShiftIdLink>(&shift_id_link))
      {
        HidEngine.startKeyShift(*key_shift_id_link);
      }
      else if (auto encoder_shift_id_link = etl::get_if<EncoderShiftIdLink>(&shift_id_link))
      {
        HidEngine.startEncoderShift(*encoder_shift_id_link);
      }
      else if (auto gesture_id_link = etl::get_if<GestureIdLink>(&shift_id_link))
      {
        HidEngine.startGesture(*gesture_id_link);
      }
    }
  }

  void Shift::onRelease()
  {
    for (auto &shift_id_link : _shift_id_links)
    {
      if (auto key_shift_id_link = etl::get_if<KeyShiftIdLink>(&shift_id_link))
      {
        HidEngine.stopKeyShift(*key_shift_id_link);
      }
      else if (auto encoder_shift_id_link = etl::get_if<EncoderShiftIdLink>(&shift_id_link))
      {
        HidEngine.stopEncoderShift(*encoder_shift_id_link);
      }
      else if (auto gesture_id_link = etl::get_if<GestureIdLink>(&shift_id_link))
      {
        HidEngine.stopGesture(*gesture_id_link);
      }
    }
  }

  ShiftIdLink Shift::IdToLink(ShiftId shift_id)
  {
    if (auto key_shift_id = etl::get_if<KeyShiftId>(&shift_id))
    {
      return KeyShiftIdLink{key_shift_id->value};
    }
    else if (auto encoder_shift_id = etl::get_if<EncoderShiftId>(&shift_id))
    {
      return EncoderShiftIdLink{encoder_shift_id->value};
    }
    else
    {
      auto gesture_id = etl::get<GestureId>(shift_id);
      return GestureIdLink{gesture_id.value};
    }
  }

} // namespace hidpg::Internal
