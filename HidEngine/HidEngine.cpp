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
#include "utility.h"

namespace hidpg
{
  namespace Internal
  {

    etl::span<Key> HidEngineClass::_keymap;
    etl::span<KeyShift> HidEngineClass::_key_shift_map;
    etl::span<Combo> HidEngineClass::_combo_map;
    etl::span<Gesture> HidEngineClass::_gesture_map;
    etl::span<Encoder> HidEngineClass::_encoder_map;
    etl::span<EncoderShift> HidEngineClass::_encoder_shift_map;

    HidEngineClass::read_pointer_delta_callback_t HidEngineClass::_read_pointer_delta_cb = nullptr;
    HidEngineClass::read_encoder_step_callback_t HidEngineClass::_read_encoder_step_cb = nullptr;

    HidEngineClass::ComboTermTimer HidEngineClass::_combo_term_timer;

    etl::intrusive_list<Key> HidEngineClass::_pressed_key_list;
    etl::intrusive_list<KeyShiftIdLink> HidEngineClass::_started_key_shift_id_list;
    etl::intrusive_list<GestureIdLink> HidEngineClass::_started_gesture_id_list;
    etl::intrusive_list<EncoderShiftIdLink> HidEngineClass::_started_encoder_shift_id_list;

    void HidEngineClass::setKeymap(etl::span<Key> keymap)
    {
      _keymap = keymap;
    }

    void HidEngineClass::setKeyShiftMap(etl::span<KeyShift> key_shift_map)
    {
      _key_shift_map = key_shift_map;
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

    void HidEngineClass::setEncoderShiftMap(etl::span<EncoderShift> encoder_shift_map)
    {
      _encoder_shift_map = encoder_shift_map;
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
      EventData evt{ApplyToKeymapEventData{key_ids}};
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::movePointer(PointingDeviceId pointing_device_id)
    {
      EventData evt{MovePointerEventData{pointing_device_id}};
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::rotateEncoder(EncoderId encoder_id)
    {
      EventData evt{RotateEncoderEventData{encoder_id}};
      HidEngineTask.enqueEvent(evt);
    }

    void HidEngineClass::setReadPointerDeltaCallback(read_pointer_delta_callback_t cb)
    {
      _read_pointer_delta_cb = cb;
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
      static etl::intrusive_list<Combo> success_combo_list;

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
            if (combo.isMatchFirstId(key_id.value()))
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
          if (combo.isMatchIds(first_commbo_id.value(), key_id.value()))
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
              auto i_item = etl::intrusive_list<Combo>::iterator(combo);
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
              auto i_item = etl::intrusive_list<Combo>::iterator(combo);
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
      for (auto &key : _pressed_key_list)
      {
        if (key.key_id == key_id)
        {
          return;
        }
      }

      BeforeOtherKeyPressEventListener::_notifyBeforeOtherKeyPress(key_id);

      auto tpl = getCurrentKey(key_id);
      auto key_shift = std::get<0>(tpl);
      auto key = std::get<1>(tpl);

      if (key == nullptr)
      {
        return;
      }

      if (key_shift != nullptr &&
          key_shift->pre_command.has_value() &&
          key_shift->pre_command.value().is_pressed == false)
      {
        key_shift->pre_command.value().is_pressed = true;
        key_shift->pre_command.value().command->press();

        if (key_shift->pre_command.value().timing == Timing::InsteadOfFirstAction)
        {
          return;
        }
      }

      _pressed_key_list.push_front(*key);
      key->command->press();
    }

    void HidEngineClass::performKeyRelease(uint8_t key_id)
    {
      for (auto &key : _pressed_key_list)
      {
        if (key.key_id == key_id)
        {
          auto i_item = etl::intrusive_list<Key>::iterator(key);
          _pressed_key_list.erase(i_item);
          key.command->release();
          return;
        }
      }
    }

    std::tuple<KeyShift *, Key *> HidEngineClass::getCurrentKey(uint8_t key_id)
    {
      for (auto &started_key_shift_id : _started_key_shift_id_list)
      {
        for (auto &key_shift : _key_shift_map)
        {
          if (key_shift.key_shift_id.value == started_key_shift_id.value)
          {
            for (auto &key : key_shift.keymap)
            {
              if (key.key_id == key_id)
              {
                return {&key_shift, &key};
              }
            }
            if (key_shift.keymap_overlay == KeymapOverlay::Disable)
            {
              return {&key_shift, nullptr};
            }
          }
        }
      }

      for (auto &key : _keymap)
      {
        if (key.key_id == key_id)
        {
          return {nullptr, &key};
        }
      }

      return {nullptr, nullptr};
    }

    void HidEngineClass::startKeyShift(KeyShiftIdLink &key_shift_id)
    {
      if (key_shift_id.is_linked())
      {
        return;
      }

      _started_key_shift_id_list.push_front(key_shift_id);

      // pre_command
      for (auto &key_shift : _key_shift_map)
      {
        if (key_shift.key_shift_id.value == key_shift_id.value &&
            key_shift.pre_command.has_value() &&
            key_shift.pre_command.value().timing == Timing::Immediately &&
            key_shift.pre_command.value().is_pressed == false)
        {
          key_shift.pre_command.value().is_pressed = true;
          key_shift.pre_command.value().command->press();
        }
      }
    }

    void HidEngineClass::stopKeyShift(KeyShiftIdLink &key_shift_id)
    {
      if (key_shift_id.is_linked() == false)
      {
        return;
      }

      auto i_item = etl::intrusive_list<KeyShiftIdLink>::iterator(key_shift_id);
      _started_key_shift_id_list.erase(i_item);
      key_shift_id.clear();

      // 同じidが同時に押されることもあり得るので最後に押されていたかチェック
      for (auto &started_key_shift_id : _started_key_shift_id_list)
      {
        if (started_key_shift_id.value == key_shift_id.value)
        {
          return;
        }
      }

      // 最後のidならclean up
      for (auto &key_shift : _key_shift_map)
      {
        if (key_shift.key_shift_id.value == key_shift_id.value &&
            key_shift.pre_command.has_value() &&
            key_shift.pre_command.value().is_pressed == true)
        {
          key_shift.pre_command.value().is_pressed = false;
          key_shift.pre_command.value().command->release();
        }
      }
    }

    //------------------------------------------------------------------+
    // MovePointer
    //------------------------------------------------------------------+
    void HidEngineClass::movePointer_impl(PointingDeviceId pointing_device_id)
    {
      if (_read_pointer_delta_cb == nullptr)
      {
        return;
      }

      Hid.waitReady();
      int16_t delta_x = 0, delta_y = 0;
      _read_pointer_delta_cb(pointing_device_id, delta_x, delta_y);

      if (delta_x == 0 && delta_y == 0)
      {
        return;
      }

      BeforeMovePointerEventListener::_notifyBeforeMovePointer(pointing_device_id, delta_x, delta_y);

      Gesture *gesture = getCurrentGesture(pointing_device_id);

      if (gesture != nullptr)
      {
        processGesture(*gesture, delta_x, delta_y);
      }
      else
      {
        Hid.mouseMove(delta_x, delta_y);
      }
    }

    Gesture *HidEngineClass::getCurrentGesture(PointingDeviceId pointing_device_id)
    {
      for (auto &started_gesture_id : _started_gesture_id_list)
      {
        for (auto &gesture : _gesture_map)
        {
          if ((started_gesture_id.value == gesture.gesture_id.value) && (gesture.pointing_device_id == pointing_device_id))
          {
            return &gesture;
          }
        }
      }

      return nullptr;
    }

    void HidEngineClass::processGesture(Gesture &gesture, int16_t delta_x, int16_t delta_y)
    {
#if (HID_ENGINE_WAIT_TIME_AFTER_INSTEAD_OF_FIRST_GESTURE_MS != 0)
      if (gesture.instead_of_first_gesture_millis.has_value())
      {
        uint32_t curr_millis = millis();
        if (static_cast<uint32_t>(curr_millis - gesture.instead_of_first_gesture_millis.value()) <= HID_ENGINE_WAIT_TIME_AFTER_INSTEAD_OF_FIRST_GESTURE_MS)
        {
          return;
        }

        gesture.instead_of_first_gesture_millis = etl::nullopt;
      }
#endif

      // 逆方向に動いたら距離をリセット
      if (bitRead(gesture.total_distance_x ^ static_cast<int32_t>(delta_x), 31))
      {
        gesture.total_distance_x = 0;
      }
      if (bitRead(gesture.total_distance_y ^ static_cast<int32_t>(delta_y), 31))
      {
        gesture.total_distance_y = 0;
      }

      // 距離を足す
      gesture.total_distance_x += delta_x;
      gesture.total_distance_y += delta_y;

      // 直近のマウスの移動距離の大きさによって実行する順序を変える
      if (abs(delta_x) >= abs(delta_y))
      {
        processGestureX(gesture);
        processGestureY(gesture);
      }
      else
      {
        processGestureY(gesture);
        processGestureX(gesture);
      }
    }

    void HidEngineClass::processGestureX(Gesture &gesture)
    {
      uint8_t n_times = std::min<int32_t>(abs(gesture.total_distance_x / gesture.distance), UINT8_MAX);

      if (n_times == 0)
      {
        return;
      }

      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.pointing_device_id);

      bool is_right = gesture.total_distance_x > 0;
      gesture.total_distance_x %= gesture.distance;

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        gesture.total_distance_y = 0;
      }

      processGesturePreCommand(gesture, n_times);

      if (n_times == 0)
      {
        return;
      }

      if (is_right)
      {
        CommandTapper.tap(gesture.right_command, n_times);
      }
      else
      {
        CommandTapper.tap(gesture.left_command, n_times);
      }
    }

    void HidEngineClass::processGestureY(Gesture &gesture)
    {
      uint8_t n_times = std::min<int32_t>(abs(gesture.total_distance_y / gesture.distance), UINT8_MAX);

      if (n_times == 0)
      {
        return;
      }

      BeforeGestureEventListener::_notifyBeforeGesture(gesture.gesture_id, gesture.pointing_device_id);

      bool is_down = gesture.total_distance_y > 0;
      gesture.total_distance_y %= gesture.distance;

      if (gesture.angle_snap == AngleSnap::Enable)
      {
        gesture.total_distance_x = 0;
      }

      processGesturePreCommand(gesture, n_times);

      if (n_times == 0)
      {
        return;
      }

      if (is_down > 0)
      {
        CommandTapper.tap(gesture.down_command, n_times);
      }
      else
      {
        CommandTapper.tap(gesture.up_command, n_times);
      }
    }

    void HidEngineClass::processGesturePreCommand(Gesture &gesture, uint8_t &n_timse)
    {
      if (gesture.pre_command.has_value() &&
          gesture.pre_command.value().is_pressed == false)
      {
        gesture.pre_command.value().is_pressed = true;
        gesture.pre_command.value().command->press();

        if (gesture.pre_command.value().timing == Timing::InsteadOfFirstAction)
        {
          n_timse--;
          gesture.instead_of_first_gesture_millis = millis();
        }
      }
    }

    void HidEngineClass::startGesture(GestureIdLink &gesture_id)
    {
      if (gesture_id.is_linked())
      {
        return;
      }

      _started_gesture_id_list.push_front(gesture_id);

      // pre_command
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id.value == gesture_id.value &&
            gesture.pre_command.has_value() &&
            gesture.pre_command.value().timing == Timing::Immediately &&
            gesture.pre_command.value().is_pressed == false)
        {
          gesture.pre_command.value().is_pressed = true;
          gesture.pre_command.value().command->press();
        }
      }
    }

    void HidEngineClass::stopGesture(GestureIdLink &gesture_id)
    {
      if (gesture_id.is_linked() == false)
      {
        return;
      }

      auto i_item = etl::intrusive_list<GestureIdLink>::iterator(gesture_id);
      _started_gesture_id_list.erase(i_item);
      gesture_id.clear();

      // 同じidが同時に押されることもあり得るので最後に押されていたかチェック
      for (auto &started_gesture_id : _started_gesture_id_list)
      {
        if (started_gesture_id.value == gesture_id.value)
        {
          return;
        }
      }

      // 最後のidならclean up
      for (auto &gesture : _gesture_map)
      {
        if (gesture.gesture_id.value == gesture_id.value)
        {
          if (gesture.pre_command.has_value() &&
              gesture.pre_command.value().is_pressed == true)
          {
            gesture.pre_command.value().is_pressed = false;
            gesture.pre_command.value().command->release();
            gesture.instead_of_first_gesture_millis = etl::nullopt;
          }
          gesture.total_distance_x = 0;
          gesture.total_distance_y = 0;
        }
      }
    }

    //------------------------------------------------------------------+
    // RotateEncoder
    //------------------------------------------------------------------+
    void HidEngineClass::rotateEncoder_impl(EncoderId encoder_id)
    {
      if (_read_encoder_step_cb == nullptr)
      {
        return;
      }

      Hid.waitReady();
      int16_t step = 0;
      _read_encoder_step_cb(encoder_id, step);

      if (step == 0)
      {
        return;
      }

      BeforeRotateEncoderEventListener::_notifyBeforeRotateEncoder(encoder_id, step);

      EncoderShift *encoder = getCurrentEncoder(encoder_id);

      if (encoder == nullptr)
      {
        return;
      }

      uint8_t step_u8 = std::min(abs(step), UINT8_MAX);

      if (encoder->pre_command.has_value() &&
          encoder->pre_command.value().is_pressed == false)
      {
        encoder->pre_command.value().is_pressed = true;
        encoder->pre_command.value().command->press();

        if (encoder->pre_command.value().timing == Timing::InsteadOfFirstAction)
        {
          step_u8--;
          if (step_u8 == 0)
          {
            return;
          }
        }
      }

      if (step > 0)
      {
        CommandTapper.tap(encoder->clockwise_command, step_u8);
      }
      else if (step < 0)
      {
        CommandTapper.tap(encoder->counterclockwise_command, step_u8);
      }
    }

    EncoderShift *HidEngineClass::getCurrentEncoder(EncoderId encoder_id)
    {
      for (auto &started_encoder_shift_id : _started_encoder_shift_id_list)
      {
        for (auto &encoder : _encoder_shift_map)
        {
          if ((started_encoder_shift_id.value == encoder.encoder_shift_id.value) && (encoder.encoder_id == encoder_id))
          {
            return &encoder;
          }
        }
      }

      for (auto &encoder : _encoder_map)
      {
        if (encoder.encoder_id == encoder_id)
        {
          return &encoder;
        }
      }

      return nullptr;
    }

    void HidEngineClass::startEncoderShift(EncoderShiftIdLink &encoder_shift_id)
    {
      if (encoder_shift_id.is_linked())
      {
        return;
      }

      _started_encoder_shift_id_list.push_front(encoder_shift_id);

      // pre_command
      for (auto &encoder : _encoder_shift_map)
      {
        if (encoder.encoder_shift_id.value == encoder_shift_id.value &&
            encoder.pre_command.has_value() &&
            encoder.pre_command.value().timing == Timing::Immediately &&
            encoder.pre_command.value().is_pressed == false)
        {
          encoder.pre_command.value().is_pressed = true;
          encoder.pre_command.value().command->press();
        }
      }
    }

    void HidEngineClass::stopEncoderShift(EncoderShiftIdLink &encoder_shift_id)
    {
      if (encoder_shift_id.is_linked() == false)
      {
        return;
      }

      auto i_item = etl::intrusive_list<EncoderShiftIdLink>::iterator(encoder_shift_id);
      _started_encoder_shift_id_list.erase(i_item);
      encoder_shift_id.clear();

      // 同じidが同時に押されることもあり得るので最後に押されていたかチェック
      for (auto &started_encoder_id : _started_encoder_shift_id_list)
      {
        if (started_encoder_id.value == encoder_shift_id.value)
        {
          return;
        }
      }

      // 最後のidならclean up
      for (auto &encoder : _encoder_shift_map)
      {
        if (encoder.encoder_shift_id.value == encoder_shift_id.value &&
            encoder.pre_command.has_value() &&
            encoder.pre_command.value().is_pressed == true)
        {
          encoder.pre_command.value().is_pressed = false;
          encoder.pre_command.value().command->release();
        }
      }
    }

  } // namespace Internal

  Internal::HidEngineClass HidEngine;

} // namespace hidpg
