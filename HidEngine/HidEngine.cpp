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

    int32_t HidEngineClass::_total_distance_x = 0;
    int32_t HidEngineClass::_total_distance_y = 0;

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
        // コンボ実行中のidは新しい入力は受け付けない
        for (auto &combo : success_combo_list)
        {
          if (combo.first_key_id == key_id || combo.second_key_id == key_id)
          {
            return;
          }
        }

        // first_id check
        if (first_commbo_id.has_value() == false)
        {
          for (auto &combo : _combo_map)
          {
            if (combo.first_key_id == key_id)
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
          if (combo.first_key_id == first_commbo_id && combo.second_key_id == key_id)
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
          if (combo.first_key_id == key_id)
          {
            combo.first_id_rereased = true;

            if (combo.first_id_rereased && combo.second_id_rereased)
            {
              combo.command->release();
              success_combo_list.erase(combo);
            }
            return;
          }

          if (combo.second_key_id == key_id)
          {
            combo.second_id_rereased = true;

            if (combo.first_id_rereased && combo.second_id_rereased)
            {
              combo.command->release();
              success_combo_list.erase(combo);
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
      static uint8_t prev_gesture_id;
      static uint8_t prev_mouse_id;

      Hid.waitReady();
      int16_t delta_x = 0, delta_y = 0;
      if (_read_mouse_delta_cb != nullptr)
      {
        _read_mouse_delta_cb(mouse_id, delta_x, delta_y);
      }

      BeforeMouseMoveEventListener::_notifyBeforeMouseMove(mouse_id, delta_x, delta_y);

      Gesture *curr_gesture = nullptr;
      GestureID *curr_gesture_id = nullptr;

      if (_started_gesture_id_list.empty() == false)
      {
        // 実行中のジェスチャーの中で一番上を取得
        curr_gesture_id = &_started_gesture_id_list.back();

        // gesture_mapからgesture_idとmouse_idが一致するアイテムを検索
        for (auto &gesture : _gesture_map)
        {
          if ((gesture.gesture_id == curr_gesture_id->getID()) && (gesture.mouse_id == mouse_id))
          {
            curr_gesture = &gesture;
            break;
          }
        }
      }

      if (curr_gesture != nullptr)
      {
        // 前回のidと違うなら距離をリセット
        if ((curr_gesture_id->getID() != prev_gesture_id) || (mouse_id != prev_mouse_id))
        {
          _total_distance_x = _total_distance_y = 0;
          prev_gesture_id = curr_gesture_id->getID();
          prev_mouse_id = mouse_id;
        }

        // 逆方向に動いたら距離をリセット
        if (bitRead(_total_distance_x ^ static_cast<int32_t>(delta_x), 31))
        {
          _total_distance_x = 0;
        }
        if (bitRead(_total_distance_y ^ static_cast<int32_t>(delta_y), 31))
        {
          _total_distance_y = 0;
        }

        // 距離を足す
        _total_distance_x += delta_x;
        _total_distance_y += delta_y;

        // 直近のマウスの移動距離の大きさによって実行する順序を変える
        if (abs(delta_x) >= abs(delta_y))
        {
          processGestureX(*curr_gesture, *curr_gesture_id);
          processGestureY(*curr_gesture, *curr_gesture_id);
        }
        else
        {
          processGestureY(*curr_gesture, *curr_gesture_id);
          processGestureX(*curr_gesture, *curr_gesture_id);
        }
      }
      else
      {
        if (!(delta_x == 0 && delta_y == 0))
        {
          Hid.mouseMove(delta_x, delta_y);
        }
      }
    }

    void HidEngineClass::processGestureX(Gesture &gesture, GestureID &gesture_id)
    {
      if (_total_distance_x <= -gesture.distance) // left
      {
        performGestureX(gesture, gesture_id, gesture.left_command);
      }
      else if (_total_distance_x >= gesture.distance) // right
      {
        performGestureX(gesture, gesture_id, gesture.right_command);
      }
    }

    void HidEngineClass::processGestureY(Gesture &gesture, GestureID &gesture_id)
    {
      if (_total_distance_y <= -gesture.distance) // up
      {
        performGestureY(gesture, gesture_id, gesture.up_command);
      }
      else if (_total_distance_y >= gesture.distance) // down
      {
        performGestureY(gesture, gesture_id, gesture.down_command);
      }
    }

    void HidEngineClass::performGestureX(Gesture &gesture, GestureID &gesture_id, Command *command)
    {
      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.mouse_id);

      if (processPreCommandInsteadOfFirstGesture(gesture, gesture_id))
      {
        _total_distance_x -= gesture.distance;
      }

      uint8_t n_times = static_cast<uint8_t>(std::min<int32_t>(abs(_total_distance_x / gesture.distance), UINT8_MAX));
      _total_distance_x %= gesture.distance;
      CommandTapper.tap(command, n_times);

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        _total_distance_y = 0;
      }
    }

    void HidEngineClass::performGestureY(Gesture &gesture, GestureID &gesture_id, Command *command)
    {
      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.mouse_id);

      if (processPreCommandInsteadOfFirstGesture(gesture, gesture_id))
      {
        _total_distance_y -= gesture.distance;
      }

      uint8_t n_times = static_cast<uint8_t>(std::min<int32_t>(abs(_total_distance_y / gesture.distance), UINT8_MAX));
      _total_distance_y %= gesture.distance;
      CommandTapper.tap(command, n_times);

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        _total_distance_x = 0;
      }
    }

    bool HidEngineClass::processPreCommandInsteadOfFirstGesture(Gesture &gesture, GestureID &gesture_id)
    {
      if (gesture.pre_command_timing == PreCommandTiming::InsteadOfFirstGesture && gesture_id.getPreCommandPressFlag() == false)
      {
        gesture_id.setPreCommandPressFlag(true);
        if (gesture.pre_command != nullptr)
        {
          gesture.pre_command->press();
        }
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

      _started_gesture_id_list.push_back(gesture_id);

      // pre_command
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id == gesture_id.getID())
        {
          if (gesture.pre_command_timing == PreCommandTiming::Immediately)
          {
            gesture_id.setPreCommandPressFlag(true);

            if (gesture.pre_command != nullptr)
            {
              gesture.pre_command->press();
            }
          }
          return;
        }
      }
    }

    void HidEngineClass::stopGesture(GestureID &gesture_id)
    {
      if (gesture_id.is_linked() == false)
      {
        return;
      }

      _started_gesture_id_list.erase(gesture_id);
      gesture_id.clear();

      if (_started_gesture_id_list.empty())
      {
        _total_distance_x = _total_distance_y = 0;
      }

      // pre_command
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id == gesture_id.getID())
        {
          if (gesture_id.getPreCommandPressFlag() == true)
          {
            gesture_id.setPreCommandPressFlag(false);

            if (gesture.pre_command != nullptr)
            {
              gesture.pre_command->release();
            }
          }
          return;
        }
      }
    }

    //------------------------------------------------------------------+
    // RotateEncoder
    //------------------------------------------------------------------+
    void HidEngineClass::rotateEncoder_impl(uint8_t encoder_id)
    {
      Encoder *curr_encoder = nullptr;

      for (auto &encoder : _encoder_map)
      {
        if (encoder.encoder_id == encoder_id)
        {
          curr_encoder = &encoder;
          break;
        }
      }

      if (curr_encoder == nullptr)
      {
        return;
      }

      if (_read_encoder_step_cb != nullptr)
      {
        int32_t step;

        Hid.waitReady();
        _read_encoder_step_cb(encoder_id, step);
        uint8_t step_u8 = constrain(abs(step), 0, UINT8_MAX);

        if (step >= 0)
        {
          CommandTapper.tap(curr_encoder->clockwise_command, step_u8);
        }
        else
        {
          CommandTapper.tap(curr_encoder->counterclockwise_command, step_u8);
        }
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
    GestureOr::GestureOr(uint8_t gesture_id, Command *command)
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
