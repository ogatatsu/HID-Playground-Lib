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

#include "HidEngine.h"
#include "ArduinoMacro.h"
#include "CommandTapper.h"
#include "HidCore.h"
#include "HidEngineTask.h"
#include <stdint.h>

extern "C"
{
  uint32_t millis();
}

namespace hidpg
{
  namespace Internal
  {

    etl::span<Key> HidEngineClass::_keymap;
    etl::span<Combo> HidEngineClass::_combo_map;
    etl::span<Gesture> HidEngineClass::_gesture_map;
    etl::span<Encoder> HidEngineClass::_encoder_map;

    HidEngineClass::read_mouse_delta_callback_t HidEngineClass::_read_mouse_delta_cb = nullptr;
    HidEngineClass::read_encoder_step_callback_t HidEngineClass::_read_encoder_step_cb = nullptr;

    HidEngineClass::ComboTermTimer HidEngineClass::_combo_term_timer;

    etl::intrusive_list<GestureID, GestureIDLink> HidEngineClass::_started_gesture_id_list;

    void HidEngineClass::setKeymap(etl::span<Key> keymap)
    {
      _keymap = keymap;
    }

    void HidEngineClass::setComboMap(etl::span<Combo> combo_map)
    {
      _combo_map = combo_map;
    }

    void HidEngineClass::setGestureMap(etl::span<Gesture> gesture_map)
    {
      _gesture_map = gesture_map;
    }

    void HidEngineClass::setEncoderMap(etl::span<Encoder> encoder_map)
    {
      _encoder_map = encoder_map;
    }

    void HidEngineClass::setHidReporter(HidReporter *hid_reporter)
    {
      Hid.setReporter(hid_reporter);
    }

    void HidEngineClass::start()
    {
      HidEngineTask.start();
    }

    void HidEngineClass::applyToKeymap(const Set &key_ids)
    {
      EventData evt;
      evt.event_type = EventType::ApplyToKeymap;
      evt.apply_to_keymap.key_ids = key_ids;
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::mouseMove(uint8_t mouse_id)
    {
      EventData evt;
      evt.event_type = EventType::MouseMove;
      evt.mouse_move.mouse_id = mouse_id;
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::rotateEncoder(uint8_t encoder_id)
    {
      EventData evt;
      evt.event_type = EventType::RotateEncoder;
      evt.rotate_encoder.encoder_id = encoder_id;
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::setReadMouseDeltaCallback(read_mouse_delta_callback_t cb)
    {
      _read_mouse_delta_cb = cb;
    }

    void HidEngineClass::setReadEncoderStepCallback(read_encoder_step_callback_t cb)
    {
      _read_encoder_step_cb = cb;
    }

    //------------------------------------------------------------------+
    // ApplyToKeymap
    //------------------------------------------------------------------+
    void HidEngineClass::applyToKeymap_impl(Set &key_ids)
    {
      static Set prev_ids;

      {
        Set press_ids = key_ids - prev_ids;

        uint8_t arr[press_ids.count()];
        press_ids.toArray(arr);

        for (uint8_t key_id : arr)
        {
          processComboAndKey(Action::Press, key_id);
        }
      }

      {
        Set release_ids = prev_ids - key_ids;

        uint8_t arr[release_ids.count()];
        release_ids.toArray(arr);

        for (uint8_t key_id : arr)
        {
          processComboAndKey(Action::Release, key_id);
        }
      }

      prev_ids = key_ids;
    }

    void HidEngineClass::processComboAndKey(Action action, etl::optional<uint8_t> key_id)
    {
      static etl::optional<uint8_t> first_commbo_id;
      static etl::intrusive_list<Combo, ComboLink> success_combo_list;

      switch (action)
      {
      case Action::Press:
      {
        // コンボ実行中のid（片方のキーだけreleaseされて再度pressされた。）の場合は新しいコンボを開始しないで通常のKeyPress
        for (auto &combo : success_combo_list)
        {
          if (combo.first_id == key_id || combo.second_id == key_id)
          {
            performKeyPress(key_id.value());
            return;
          }
        }

        // first_id check
        if (first_commbo_id.has_value() == false)
        {
          for (auto &combo : _combo_map)
          {
            if (combo.isMatchFirstID(key_id.value()))
            {
              // first_id success
              first_commbo_id = key_id;
              startComboTermTimer(combo.combo_term_ms);
              return;
            }
          }
          // first_id failure
          performKeyPress(key_id.value());
          return;
        }

        // second_id check
        for (auto &combo : _combo_map)
        {
          if (combo.isMatchIDs(first_commbo_id.value(), key_id.value()))
          {
            // combo success
            combo.first_id_rereased = false;
            combo.second_id_rereased = false;
            combo.command->press();
            success_combo_list.push_back(combo);
            first_commbo_id = etl::nullopt;
            return;
          }
        }

        // second_id failure
        performKeyPress(first_commbo_id.value());
        performKeyPress(key_id.value());
        first_commbo_id = etl::nullopt;
      }
      break;

      case Action::Release:
      {
        // combo実行中のidがreleaseされた場合
        for (auto &combo : success_combo_list)
        {
          if (combo.first_id == key_id && combo.first_id_rereased == false)
          {
            combo.first_id_rereased = true;

            if ((combo.isFastRelease() && combo.second_id_rereased == false) ||
                (combo.isSlowRelease() && combo.second_id_rereased))
            {
              combo.command->release();
            }

            if (combo.second_id_rereased)
            {
              auto i_item = etl::intrusive_list<Combo, ComboLink>::iterator(combo);
              success_combo_list.erase(i_item);
            }
            return;
          }

          if (combo.second_id == key_id && combo.second_id_rereased == false)
          {
            combo.second_id_rereased = true;

            if ((combo.isFastRelease() && combo.first_id_rereased == false) ||
                (combo.isSlowRelease() && combo.first_id_rereased))
            {
              combo.command->release();
            }

            if (combo.first_id_rereased)
            {
              auto i_item = etl::intrusive_list<Combo, ComboLink>::iterator(combo);
              success_combo_list.erase(i_item);
            }
            return;
          }
        }

        // first_idがタップされた場合
        if (first_commbo_id == key_id)
        {
          first_commbo_id = etl::nullopt;
          performKeyPress(key_id.value());
          performKeyRelease(key_id.value());
          return;
        }

        // first_idより前に押されていたidがreleaseされた場合
        if (first_commbo_id.has_value())
        {
          performKeyPress(first_commbo_id.value());
          performKeyRelease(key_id.value());
          first_commbo_id = etl::nullopt;
          return;
        }

        // 他
        performKeyRelease(key_id.value());
      }
      break;

      case Action::ComboTermTimer:
      {
        if (first_commbo_id.has_value())
        {
          performKeyPress(first_commbo_id.value());
          first_commbo_id = etl::nullopt;
        }
      }
      break;

      default:
        break;
      }
    }

    void HidEngineClass::performKeyPress(uint8_t key_id)
    {
      for (auto &key : _keymap)
      {
        if (key.key_id == key_id)
        {
          key.command->press();
          return;
        }
      }
    }

    void HidEngineClass::performKeyRelease(uint8_t key_id)
    {
      for (auto &key : _keymap)
      {
        if (key.key_id == key_id)
        {
          key.command->release();
          return;
        }
      }
    }

    //------------------------------------------------------------------+
    // MouseMove
    //------------------------------------------------------------------+
    void HidEngineClass::mouseMove_impl(uint8_t mouse_id)
    {
      if (_read_mouse_delta_cb == nullptr)
      {
        return;
      }

      Hid.waitReady();
      int16_t delta_x = 0, delta_y = 0;
      _read_mouse_delta_cb(mouse_id, delta_x, delta_y);

      if (delta_x == 0 && delta_y == 0)
      {
        return;
      }

      BeforeMouseMoveEventListener::_notifyBeforeMouseMove(mouse_id, delta_x, delta_y);

      // gesture_mapからgesture_idとmouse_idが一致するアイテムを検索
      Gesture *gesture = nullptr;
      for (auto &started_id : _started_gesture_id_list)
      {
        for (auto &gst : _gesture_map)
        {
          if ((started_id.id == gst.gesture_id) && (gst.mouse_id == mouse_id))
          {
            gesture = &gst;
            break;
          }
        }
      }

      if (gesture != nullptr)
      {
        if (gesture->instead_of_first_gesture_millis.has_value())
        {
          uint32_t curr_millis = millis();
          if (static_cast<uint32_t>(curr_millis - gesture->instead_of_first_gesture_millis.value()) <= HID_ENGINE_WAIT_TIME_AFTER_INSTEAD_OF_FIRST_GESTURE_MS)
          {
            return;
          }

          gesture->instead_of_first_gesture_millis = etl::nullopt;
        }

        // 逆方向に動いたら距離をリセット
        if (bitRead(gesture->total_distance_x ^ static_cast<int32_t>(delta_x), 31))
        {
          gesture->total_distance_x = 0;
        }
        if (bitRead(gesture->total_distance_y ^ static_cast<int32_t>(delta_y), 31))
        {
          gesture->total_distance_y = 0;
        }

        // 距離を足す
        gesture->total_distance_x += delta_x;
        gesture->total_distance_y += delta_y;

        // 直近のマウスの移動距離の大きさによって実行する順序を変える
        if (abs(delta_x) >= abs(delta_y))
        {
          processGestureX(*gesture);
          processGestureY(*gesture);
        }
        else
        {
          processGestureY(*gesture);
          processGestureX(*gesture);
        }
      }
      else
      {
        Hid.mouseMove(delta_x, delta_y);
      }
    }

    void HidEngineClass::processGestureX(Gesture &gesture)
    {
      if (gesture.total_distance_x <= -gesture.distance) // left
      {
        performGestureX(gesture, gesture.left_command);
      }
      else if (gesture.total_distance_x >= gesture.distance) // right
      {
        performGestureX(gesture, gesture.right_command);
      }
    }

    void HidEngineClass::processGestureY(Gesture &gesture)
    {
      if (gesture.total_distance_y <= -gesture.distance) // up
      {
        performGestureY(gesture, gesture.up_command);
      }
      else if (gesture.total_distance_y >= gesture.distance) // down
      {
        performGestureY(gesture, gesture.down_command);
      }
    }

    void HidEngineClass::performGestureX(Gesture &gesture, Command *command)
    {
      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.mouse_id);

      if (processPreCommandInsteadOfFirstGesture(gesture))
      {
        gesture.total_distance_x -= gesture.distance;
      }

      processPreCommandJustBeforeFirstGesture(gesture);

      uint8_t n_times = static_cast<uint8_t>(std::min<int32_t>(abs(gesture.total_distance_x / gesture.distance), UINT8_MAX));
      gesture.total_distance_x %= gesture.distance;
      CommandTapper.tap(command, n_times);

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        gesture.total_distance_y = 0;
      }
    }

    void HidEngineClass::performGestureY(Gesture &gesture, Command *command)
    {
      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.mouse_id);

      if (processPreCommandInsteadOfFirstGesture(gesture))
      {
        gesture.total_distance_y -= gesture.distance;
      }

      processPreCommandJustBeforeFirstGesture(gesture);

      uint8_t n_times = static_cast<uint8_t>(std::min<int32_t>(abs(gesture.total_distance_y / gesture.distance), UINT8_MAX));
      gesture.total_distance_y %= gesture.distance;
      CommandTapper.tap(command, n_times);

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        gesture.total_distance_x = 0;
      }
    }

    void HidEngineClass::processPreCommandJustBeforeFirstGesture(Gesture &gesture)
    {
      if (gesture.is_pre_command_pressed == false && gesture.pre_command_timing == PreCommandTiming::JustBeforeFirstGesture)
      {
        gesture.is_pre_command_pressed = true;
        if (gesture.pre_command != nullptr)
        {
          gesture.pre_command->press();
        }
      }
    }

    bool HidEngineClass::processPreCommandInsteadOfFirstGesture(Gesture &gesture)
    {
      if (gesture.is_pre_command_pressed == false && gesture.pre_command_timing == PreCommandTiming::InsteadOfFirstGesture)
      {
        gesture.is_pre_command_pressed = true;
        if (gesture.pre_command != nullptr)
        {
          gesture.pre_command->press();
        }
        gesture.instead_of_first_gesture_millis = millis();
        return true;
      }

      return false;
    }

    void HidEngineClass::startGesture(GestureID &gesture_id)
    {
      if (gesture_id.is_linked())
      {
        return;
      }

      _started_gesture_id_list.push_front(gesture_id);

      // pre_command
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id == gesture_id.id)
        {
          if (gesture.pre_command_timing == PreCommandTiming::Immediately &&
              gesture.is_pre_command_pressed == false)
          {
            gesture.is_pre_command_pressed = true;
            if (gesture.pre_command != nullptr)
            {
              gesture.pre_command->press();
            }
          }
        }
      }
    }

    void HidEngineClass::stopGesture(GestureID &gesture_id)
    {
      if (gesture_id.is_linked() == false)
      {
        return;
      }

      auto i_item = etl::intrusive_list<GestureID, GestureIDLink>::iterator(gesture_id);
      _started_gesture_id_list.erase(i_item);
      gesture_id.clear();

      // 同じidが同時に押されることもあり得るので最後に押されていたかチェック
      for (auto &started_id : _started_gesture_id_list)
      {
        if (started_id.id == gesture_id.id)
        {
          return;
        }
      }

      // 最後のidならclean up
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id == gesture_id.id)
        {
          if (gesture.is_pre_command_pressed == true)
          {
            gesture.is_pre_command_pressed = false;
            if (gesture.pre_command != nullptr)
            {
              gesture.pre_command->release();
            }
          }
          gesture.total_distance_x = 0;
          gesture.total_distance_y = 0;
          gesture.instead_of_first_gesture_millis = etl::nullopt;
        }
      }
    }

    //------------------------------------------------------------------+
    // RotateEncoder
    //------------------------------------------------------------------+
    void HidEngineClass::rotateEncoder_impl(uint8_t encoder_id)
    {
      if (_read_encoder_step_cb == nullptr)
      {
        return;
      }

      Encoder *encoder = nullptr;
      for (auto &enc : _encoder_map)
      {
        if (enc.encoder_id == encoder_id)
        {
          encoder = &enc;
          break;
        }
      }

      if (encoder == nullptr)
      {
        return;
      }

      Hid.waitReady();
      int16_t step = 0;
      _read_encoder_step_cb(encoder_id, step);
      uint8_t step_u8 = std::min(abs(step), UINT8_MAX);

      if (step > 0)
      {
        CommandTapper.tap(encoder->clockwise_command, step_u8);
      }
      else if (step < 0)
      {
        CommandTapper.tap(encoder->counterclockwise_command, step_u8);
      }
    }

    //------------------------------------------------------------------+
    // GestureCommand
    //------------------------------------------------------------------+
    GestureCommand::GestureCommand(uint8_t gesture_id) : _gesture_id(gesture_id)
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
    GestureOr::GestureOr(uint8_t gesture_id, NotNullCommandPtr command)
        : BeforeOtherCommandPressEventListener(this), BeforeGestureEventListener(), _gesture_id(gesture_id), _command(command), _state(State::Unexecuted)
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

    void GestureOr::onBeforeGesture(uint8_t gesture_id, uint8_t mouse_id)
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
    GestureOrNK::GestureOrNK(uint8_t gesture_id, KeyCode key_code)
        : BeforeOtherCommandPressEventListener(this), BeforeGestureEventListener(), _gesture_id(gesture_id), _nk_command(key_code), _state(State::Unexecuted)
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

    void GestureOrNK::onBeforeGesture(uint8_t gesture_id, uint8_t mouse_id)
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

  } // namespace Internal

  Internal::HidEngineClass HidEngine;

} // namespace hidpg
