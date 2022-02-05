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
#include "Arduino.h"
#include "CommandTapper.h"
#include "HidCore.h"
#include "HidEngineTask.h"

namespace hidpg
{
  namespace Internal
  {

    Key *HidEngineClass::_keymap = nullptr;
    SequenceKey *HidEngineClass::_sequence_keymap = nullptr;
    Gesture *HidEngineClass::_gesture_map = nullptr;
    Encoder *HidEngineClass::_encoder_map = nullptr;

    uint8_t HidEngineClass::_keymap_len = 0;
    uint8_t HidEngineClass::_sequence_keymap_len = 0;
    uint8_t HidEngineClass::_gesture_map_len = 0;
    uint8_t HidEngineClass::_encoder_map_len = 0;

    HidEngineClass::read_mouse_delta_callback_t HidEngineClass::_read_mouse_delta_cb = nullptr;
    HidEngineClass::read_encoder_step_callback_t HidEngineClass::_read_encoder_step_cb = nullptr;

    HidEngineClass::SequenceModeState HidEngineClass::_sequence_mode_state = HidEngineClass::SequenceModeState::Disable;
    etl::intrusive_list<GestureID, GestureIDLink> HidEngineClass::_gesture_list;

    int32_t HidEngineClass::_total_distance_x = 0;
    int32_t HidEngineClass::_total_distance_y = 0;

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

    size_t HidEngineClass::getValidLength(const uint8_t key_ids[], size_t max_len)
    {
      size_t i = 0;
      for (; i < max_len; i++)
      {
        if (key_ids[i] == 0)
        {
          break;
        }
      }
      return i;
    }

    //------------------------------------------------------------------+
    // ApplyToKeymap
    //------------------------------------------------------------------+
    void HidEngineClass::applyToKeymap_impl(Set &key_ids)
    {
      processSequenceKeymap(key_ids);
      processKeymap(key_ids);
    }

    void HidEngineClass::processSequenceKeymap(Set &key_ids)
    {
      static Set prev_ids, pressed_in_sequence_mode_ids;
      static uint8_t id_seq[HID_ENGINE_MAX_SEQUENCE_COUNT];
      static size_t id_seq_len = 0;
      static SequenceKey *matched;

      // シーケンスモード中に押されたidがリリースされるまで監視する
      if (pressed_in_sequence_mode_ids.count() != 0)
      {
        // リリースされたids = 1つ前のids - 現在のids
        Set release_ids = prev_ids - key_ids;
        pressed_in_sequence_mode_ids -= release_ids;
      }

      if (_sequence_mode_state == SequenceModeState::MatchProcess)
      {
        // 新しく押されたidを順番に保存していきsequence_keymap内で一致している物があるかどうかを調べる
        // 新しく押されたids = 現在のids - 1つ前のids
        Set new_press_ids = key_ids - prev_ids;

        // Setだと直接値を取得できないので配列に変換
        uint16_t new_press_len = new_press_ids.count();
        uint8_t new_press_arr[new_press_len];
        new_press_ids.toArray(new_press_arr);

        // id_seqにシーケンスモードになってから新しく押されたidを順番に保存する
        size_t i = 0;
        while ((id_seq_len < HID_ENGINE_MAX_SEQUENCE_COUNT) && (i < new_press_len))
        {
          id_seq[id_seq_len++] = new_press_arr[i++];
        }

        // 順番に保存したidとsequence_keymapの比較
        MatchResult match_result = matchWithSequenceKeymap(id_seq, id_seq_len, &matched);

        if (match_result == MatchResult::NoMatch)
        { // マッチしなければシーケンスモードを解除
          id_seq_len = 0;
          _sequence_mode_state = SequenceModeState::Disable;
        }
        else if (match_result == MatchResult::PartialMatch)
        { // 部分マッチならば何もしない
          // pass
        }
        else if (match_result == MatchResult::Match)
        { // 完全マッチしたらコマンドを実行してStateをWaitReleaseに移行
          id_seq_len = 0;
          matched->command->press();
          _sequence_mode_state = SequenceModeState::WaitRelease;
        }

        // シーケンスモード時に押されたidは1回リリースされるまではkeymapのコマンドを実行しない
        // そのためリリースされるまで監視する必要があるのでidを追加していく
        pressed_in_sequence_mode_ids |= new_press_ids;
      }
      else if (_sequence_mode_state == SequenceModeState::WaitRelease)
      {
        // 完全マッチ時に実行したコマンドを解除
        // key_ids内からマッチしたid列の最後のid無くなったら解除する
        if (key_ids.contains(matched->key_ids[matched->key_ids_len - 1]) == false)
        {
          matched->command->release();
          _sequence_mode_state = SequenceModeState::Disable;
        }
      }

      // 1つ前のidとして保存
      prev_ids = key_ids;

      if (pressed_in_sequence_mode_ids.count() != 0)
      {
        // シーケンスモード中に押されたidはprocessKeymap()では処理しない
        key_ids -= pressed_in_sequence_mode_ids;
      }
    }

    HidEngineClass::MatchResult HidEngineClass::matchWithSequenceKeymap(const uint8_t id_seq[], size_t len, SequenceKey **matched)
    {
      for (size_t i = 0; i < _sequence_keymap_len; i++)
      {
        size_t min_len = min(len, _sequence_keymap[i].key_ids_len);
        if (memcmp(id_seq, _sequence_keymap[i].key_ids, min_len) == 0)
        {
          if (len == _sequence_keymap[i].key_ids_len)
          {
            *matched = &_sequence_keymap[i];
            return MatchResult::Match;
          }
          return MatchResult::PartialMatch;
        }
      }
      return MatchResult::NoMatch;
    }

    void HidEngineClass::processKeymap(Set &key_ids)
    {
      for (size_t i = 0; i < _keymap_len; i++)
      {
        // idが押されているかを取得
        bool pressed = key_ids.contains(_keymap[i].key_id);

        // コマンドに現在の状態を適用する
        if (pressed)
        {
          _keymap[i].command->press();
        }
        else
        {
          _keymap[i].command->release();
        }
      }
    }

    void HidEngineClass::switchSequenceMode()
    {
      if (_sequence_mode_state == SequenceModeState::Disable)
      {
        _sequence_mode_state = SequenceModeState::MatchProcess;
      }
    }

    //------------------------------------------------------------------+
    // MouseMove
    //------------------------------------------------------------------+
    void HidEngineClass::mouseMove_impl(uint8_t mouse_id)
    {
      static uint8_t prev_gesture_id;
      static uint8_t prev_mouse_id;

      BmmEventListener::_notifyBeforeMouseMove();

      Gesture gesture;
      bool is_gesturing = false;

      if (_gesture_list.size() > 0)
      {
        // 一番上のgesture_idを取得
        uint8_t gesture_id = _gesture_list.back().getID();

        // gesture_mapからgesture_idとmouse_idが一致するアイテムを検索
        for (int i = 0; i < _gesture_map_len; i++)
        {
          if ((_gesture_map[i].gesture_id == gesture_id) && (_gesture_map[i].mouse_id == mouse_id))
          {
            gesture = _gesture_map[i];
            is_gesturing = true;
            break;
          }
        }
      }

      if (is_gesturing)
      {
        // 前回のidと違うなら距離をリセット
        if ((gesture.gesture_id != prev_gesture_id) || (mouse_id != prev_mouse_id))
        {
          _total_distance_x = _total_distance_y = 0;
          prev_gesture_id = gesture.gesture_id;
          prev_mouse_id = mouse_id;
        }

        int16_t delta_x = 0, delta_y = 0;

        if (_read_mouse_delta_cb != nullptr)
        {
          // 距離を取得
          _read_mouse_delta_cb(mouse_id, delta_x, delta_y);

          // 逆方向に動いたら距離をリセット
          if (bitRead(_total_distance_x ^ delta_x, 15))
          {
            _total_distance_x = 0;
          }
          if (bitRead(_total_distance_y ^ delta_y, 15))
          {
            _total_distance_y = 0;
          }

          //距離を足す
          _total_distance_x += delta_x;
          _total_distance_y += delta_y;
        }

        // 距離の大きさによって実行する順序を変える
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
      else
      {
        if (_read_mouse_delta_cb != nullptr)
        {
          int16_t delta_x, delta_y;

          Hid.waitReady();
          _read_mouse_delta_cb(mouse_id, delta_x, delta_y);

          if (!(delta_x == 0 && delta_y == 0))
          {
            Hid.mouseMove(delta_x, delta_y);
          }
        }
      }
    }

    void HidEngineClass::processGestureY(Gesture &gesture)
    {
      int16_t threshold = gesture.distance;

      if (_total_distance_y <= -threshold) // up
      {
        BgstEventListener::_notifyBeforeGesture();

        uint8_t n_times = min(abs(_total_distance_y / threshold), UINT8_MAX);
        _total_distance_y %= threshold;
        CommandTapper.tap(gesture.up_command, n_times);
        if (gesture.angle_snap == AngleSnap::Enable)
        {
          _total_distance_x = 0;
        }
      }
      else if (_total_distance_y >= threshold) // down
      {
        BgstEventListener::_notifyBeforeGesture();

        uint8_t n_times = min(_total_distance_y / threshold, UINT8_MAX);
        _total_distance_y %= threshold;
        CommandTapper.tap(gesture.down_command, n_times);
        if (gesture.angle_snap == AngleSnap::Enable)
        {
          _total_distance_x = 0;
        }
      }
    }

    void HidEngineClass::processGestureX(Gesture &gesture)
    {
      int16_t threshold = gesture.distance;

      if (_total_distance_x <= -threshold) // left
      {
        BgstEventListener::_notifyBeforeGesture();

        uint8_t n_times = min(abs(_total_distance_x / threshold), UINT8_MAX);
        _total_distance_x %= threshold;
        CommandTapper.tap(gesture.left_command, n_times);
        if (gesture.angle_snap == AngleSnap::Enable)
        {
          _total_distance_y = 0;
        }
      }
      else if (_total_distance_x >= threshold) // right
      {
        BgstEventListener::_notifyBeforeGesture();

        uint8_t n_times = min(_total_distance_x / threshold, UINT8_MAX);
        _total_distance_x %= threshold;
        CommandTapper.tap(gesture.right_command, n_times);
        if (gesture.angle_snap == AngleSnap::Enable)
        {
          _total_distance_y = 0;
        }
      }
    }

    void HidEngineClass::startGesture(GestureID &gesture_id)
    {
      if (gesture_id.is_linked() == false)
      {
        _gesture_list.push_back(gesture_id);
      }
    }

    void HidEngineClass::stopGesture(GestureID &gesture_id)
    {
      if (gesture_id.is_linked())
      {
        auto i_item = etl::intrusive_list<GestureID, GestureIDLink>::iterator(gesture_id);
        _gesture_list.erase(i_item);
        gesture_id.clear();
        if (_gesture_list.empty())
        {
          _total_distance_x = _total_distance_y = 0;
        }
      }
    }

    //------------------------------------------------------------------+
    // RotateEncoder
    //------------------------------------------------------------------+
    void HidEngineClass::rotateEncoder_impl(uint8_t encoder_id)
    {
      // encoderMapから一致するencoder_idのインデックスを検索
      int idx = -1;
      for (int i = 0; i < _encoder_map_len; i++)
      {
        if (_encoder_map[i].encoder_id == encoder_id)
        {
          idx = i;
          break;
        }
      }

      // 一致するencoder_idが無かったら抜ける
      if (idx == -1)
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
          CommandTapper.tap(_encoder_map[idx].clockwise_command, step_u8);
        }
        else
        {
          CommandTapper.tap(_encoder_map[idx].counterclockwise_command, step_u8);
        }
      }
    }

    //------------------------------------------------------------------+
    // SequenceMode
    //------------------------------------------------------------------+
    void SequenceMode::onPress(uint8_t n_times)
    {
      HidEngine.switchSequenceMode();
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
    // GestureOrTap
    //------------------------------------------------------------------+
    GestureOrTap::GestureOrTap(uint8_t gesture_id, Command *command)
        : BdrcpEventListener(this), BgstEventListener(), _gesture_id(gesture_id), _command(command), _state(State::Unexecuted)
    {
      _command->setParent(this);
    }

    void GestureOrTap::onPress(uint8_t n_times)
    {
      HidEngine.startGesture(_gesture_id);
      _state = State::Pressed;
      startListen();
    }

    uint8_t GestureOrTap::onRelease()
    {
      if (_state == State::Pressed)
      {
        HidEngine.stopGesture(_gesture_id);
        CommandTapper.tap(_command);
        stopListen();
      }
      else if (_state == State::DifferentCommandPressed)
      {
        _command->release();
      }
      else if (_state == State::Gestured)
      {
        HidEngine.stopGesture(_gesture_id);
      }

      _state = State::Unexecuted;
      return 1;
    }

    void GestureOrTap::onBeforeDifferentRootCommandPress()
    {
      if (_state == State::Pressed)
      {
        HidEngine.stopGesture(_gesture_id);
        _command->press();
        _state = State::DifferentCommandPressed;
        stopListen();
      }
    }

    void GestureOrTap::onBeforeGesture()
    {
      if (_state == State::Pressed)
      {
        _state = State::Gestured;
        stopListen();
      }
    }

    void GestureOrTap::startListen()
    {
      startListen_BeforeDifferentRootCommandPress();
      startListen_BeforeGesture();
    }

    void GestureOrTap::stopListen()
    {
      stopListen_BeforeDifferentRootCommandPress();
      stopListen_BeforeGesture();
    }

  } // namespace Internal

  Internal::HidEngineClass HidEngine;

} // namespace hidpg
