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

  Key *HidEngineClass::_keymap = nullptr;
  SimulKey *HidEngineClass::_simul_keymap = nullptr;
  SeqKey *HidEngineClass::_seq_keymap = nullptr;
  Track *HidEngineClass::_track_map = nullptr;
  Encoder *HidEngineClass::_encoder_map = nullptr;

  uint8_t HidEngineClass::_keymap_len = 0;
  uint8_t HidEngineClass::_simul_keymap_len = 0;
  uint8_t HidEngineClass::_seq_keymap_len = 0;
  uint8_t HidEngineClass::_track_map_len = 0;
  uint8_t HidEngineClass::_encoder_map_len = 0;

  HidEngineClass::read_mouse_delta_callback_t HidEngineClass::_read_mouse_delta_cb = nullptr;
  HidEngineClass::read_encoder_step_callback_t HidEngineClass::_read_encoder_step_cb = nullptr;

  HidEngineClass::SeqModeState HidEngineClass::_seq_mode_state = HidEngineClass::SeqModeState::Disable;
  LinkedList<HidEngineClass::Tracking *> HidEngineClass::_tracking_list;
  int32_t HidEngineClass::_distance_x = 0;
  int32_t HidEngineClass::_distance_y = 0;

  HidEngineClass HidEngine;

  void HidEngineClass::setHidReporter(HidReporter *hid_reporter)
  {
    Hid.setReporter(hid_reporter);
  }

  void HidEngineClass::begin()
  {
    HidEngineTask.begin();
    CommandTapper.begin();
  }

  void HidEngineClass::applyToKeymap(const Set &key_ids)
  {
    EventData evt;
    evt.event_type = EventType::ApplyToKeymap;
    evt.apply_to_keymap.key_ids = key_ids;
    HidEngineTask.enqueEvent(evt);
  }

  void HidEngineClass::mouseMove()
  {
    EventData evt;
    evt.event_type = EventType::MouseMove;
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
    processSeqKeymap(key_ids);
    processSimulKeymap(key_ids);
    processKeymap(key_ids);
  }

  void HidEngineClass::processSeqKeymap(Set &key_ids)
  {
    static Set prev_ids, pressed_in_seq_mode_ids;
    static uint8_t id_seq[HID_ENGINE_MAX_SEQ_COUNT];
    static size_t id_seq_len = 0;
    static SeqKey *matched;

    // シーケンスモード中に押されたidがリリースされるまで監視する
    if (pressed_in_seq_mode_ids.count() != 0)
    {
      // リリースされたids = 1つ前のids - 現在のids
      Set release_ids = prev_ids - key_ids;
      pressed_in_seq_mode_ids -= release_ids;
    }

    if (_seq_mode_state == SeqModeState::Running)
    {
      // 新しく押されたidを順番に保存していきseq_keymap内で一致している物があるかどうかを調べる
      // 新しく押されたids = 現在のids - 1つ前のids
      Set new_press_ids = key_ids - prev_ids;

      // Setだと直接値を取得できないので配列に変換
      uint16_t new_press_len = new_press_ids.count();
      uint8_t new_press_arr[new_press_len];
      new_press_ids.toArray(new_press_arr);

      // id_seqにシーケンスモードになってから新しく押されたidを順番に保存する
      size_t i = 0;
      while ((id_seq_len < HID_ENGINE_MAX_SEQ_COUNT) && (i < new_press_len))
      {
        id_seq[id_seq_len++] = new_press_arr[i++];
      }

      // 順番に保存したidとseq_keymapの比較
      int match_result = match_with_seqKeymap(id_seq, id_seq_len, &matched);

      if (match_result == 0)
      { // マッチしなければシーケンスモードを解除
        id_seq_len = 0;
        _seq_mode_state = SeqModeState::Disable;
      }
      else if (match_result == 1)
      { // 部分マッチならば何もしない
        // pass
      }
      else if (match_result == 2)
      { // 完全マッチしたらコマンドを実行してStateをWaitReleaseに移行
        id_seq_len = 0;
        matched->command->press();
        _seq_mode_state = SeqModeState::WaitRelease;
      }

      // シーケンスモード時に押されたidは1回リリースされるまではkeymap,simul_keymapのコマンドを実行しない
      // そのためリリースされるまで監視する必要があるのでidを追加していく
      pressed_in_seq_mode_ids |= new_press_ids;
    }
    else if (_seq_mode_state == SeqModeState::WaitRelease)
    {
      // 完全マッチ時に実行したコマンドを解除
      // key_ids内からマッチしたid列の最後のid無くなったら解除する
      if (key_ids.contains(matched->key_ids[matched->key_ids_len - 1]) == false)
      {
        matched->command->release();
        _seq_mode_state = SeqModeState::Disable;
      }
    }

    // 1つ前のidとして保存
    prev_ids = key_ids;

    if (pressed_in_seq_mode_ids.count() != 0)
    {
      // シーケンスモード中に押されたidはprocessKeymap()では処理しない
      key_ids -= pressed_in_seq_mode_ids;
    }
  }

  // 引数のid_seqがseq_keymapの定義とマッチするか調べる
  // 戻り値が
  // 0: マッチしない
  // 1: 部分マッチ
  // 2: 完全にマッチ、完全にマッチした場合はmatchedにマッチしたSeqKeyを入れて返す
  int HidEngineClass::match_with_seqKeymap(const uint8_t id_seq[], size_t len, SeqKey **matched)
  {
    for (size_t i = 0; i < _seq_keymap_len; i++)
    {
      size_t min_len = min(len, _seq_keymap[i].key_ids_len);
      if (memcmp(id_seq, _seq_keymap[i].key_ids, min_len) == 0)
      {
        if (len == _seq_keymap[i].key_ids_len)
        {
          *matched = &_seq_keymap[i];
          return 2;
        }
        return 1;
      }
    }
    return 0;
  }

  void HidEngineClass::processSimulKeymap(Set &key_ids)
  {
    for (size_t i = 0; i < _simul_keymap_len; i++)
    {
      // 全てのidが押されているかを取得
      bool pressed = key_ids.containsAll(_simul_keymap[i].key_ids, _simul_keymap[i].key_ids_len);

      // コマンドに現在の状態を適用する
      if (pressed)
      {
        _simul_keymap[i].command->press();
      }
      else
      {
        _simul_keymap[i].command->release();
      }
    }
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

  //------------------------------------------------------------------+
  // MouseMove
  //------------------------------------------------------------------+
  void HidEngineClass::mouseMove_impl()
  {
    static uint8_t prev_track_id;

    Command::_notifyBeforeMouseMove();

    if (_tracking_list.size() > 0) // Tracking Commandが実行中の場合
    {
      // 一番上のtrack_idを取得
      uint8_t track_id = _tracking_list[0]->getID();

      // 前回のidと違うなら0から距離を測る
      if (track_id != prev_track_id)
      {
        _distance_x = _distance_y = 0;
        prev_track_id = track_id;
      }

      int16_t delta_x = 0, delta_y = 0;

      if (_read_mouse_delta_cb != nullptr)
      {
        _read_mouse_delta_cb(&delta_x, &delta_y); // 距離を足す
        _distance_x += delta_x;
        _distance_y += delta_y;
      }

      // trackMapから一致するtrack_idのインデックスを検索
      int match_idx = -1;
      for (int i = 0; i < _track_map_len; i++)
      {
        if (_track_map[i].track_id == track_id)
        {
          match_idx = i;
          break;
        }
      }

      // 一致するtrack_idが無かったら抜ける
      if (match_idx == -1)
      {
        return;
      }

      // 距離の大きさによって実行する順序を変える
      if (abs(delta_x) >= abs(delta_y))
      {
        processTrackX(match_idx);
        processTrackY(match_idx);
      }
      else
      {
        processTrackY(match_idx);
        processTrackX(match_idx);
      }
    }
    else
    {
      if (_read_mouse_delta_cb != nullptr)
      {
        int16_t x, y;

        Hid.waitReady();
        _read_mouse_delta_cb(&x, &y);

        if (!(x == 0 && y == 0))
        {
          Hid.mouseMove(x, y);
        }
      }
    }
  }

  void HidEngineClass::processTrackY(size_t track_map_idx)
  {
    int16_t threshold = _track_map[track_map_idx].threshold_distance;

    if (_distance_y <= -threshold) // 上
    {
      uint8_t n_times = min(abs(_distance_y / threshold), UINT8_MAX);
      _distance_y %= threshold;
      CommandTapper.tap(_track_map[track_map_idx].up_command, n_times);
      if (_track_map[track_map_idx].angle_snap == AngleSnap::Enable)
      {
        _distance_x = 0;
      }
    }
    else if (_distance_y >= threshold) // 下
    {
      uint8_t n_times = min(_distance_y / threshold, UINT8_MAX);
      _distance_y %= threshold;
      CommandTapper.tap(_track_map[track_map_idx].down_command, n_times);
      if (_track_map[track_map_idx].angle_snap == AngleSnap::Enable)
      {
        _distance_x = 0;
      }
    }
  }

  void HidEngineClass::processTrackX(size_t track_map_idx)
  {
    int16_t threshold = _track_map[track_map_idx].threshold_distance;

    if (_distance_x <= -threshold) // 左
    {
      uint8_t n_times = min(abs(_distance_x / threshold), UINT8_MAX);
      _distance_x %= threshold;
      CommandTapper.tap(_track_map[track_map_idx].left_command, n_times);
      if (_track_map[track_map_idx].angle_snap == AngleSnap::Enable)
      {
        _distance_y = 0;
      }
    }
    else if (_distance_x >= threshold) // 右
    {
      uint8_t n_times = min(_distance_x / threshold, UINT8_MAX);
      _distance_x %= threshold;
      CommandTapper.tap(_track_map[track_map_idx].right_command, n_times);
      if (_track_map[track_map_idx].angle_snap == AngleSnap::Enable)
      {
        _distance_y = 0;
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
      _read_encoder_step_cb(encoder_id, &step);
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
  // HidEngine inner command
  //------------------------------------------------------------------+
  void HidEngineClass::switchSequenceMode()
  {
    if (_seq_mode_state == SeqModeState::Disable)
    {
      _seq_mode_state = SeqModeState::Running;
    }
  }

  void HidEngineClass::startTracking(HidEngineClass::Tracking *tracking)
  {
    _tracking_list.unshift(tracking);
  }

  void HidEngineClass::stopTracking(HidEngineClass::Tracking *tracking)
  {
    for (int i = 0; i < _tracking_list.size(); i++)
    {
      if (_tracking_list.get(i) == tracking)
      {
        _tracking_list.remove(i);
        if (_tracking_list.size() == 0)
        {
          _distance_x = _distance_y = 0;
        }
        return;
      }
    }
  }

  //------------------------------------------------------------------+
  // SequenceMode
  //------------------------------------------------------------------+
  void HidEngineClass::SequenceMode::onPress(uint8_t n_times)
  {
    HidEngineClass::switchSequenceMode();
  }

  //------------------------------------------------------------------+
  // Tracking
  //------------------------------------------------------------------+
  HidEngineClass::Tracking::Tracking(uint8_t track_id) : _track_id(track_id)
  {
  }

  uint8_t HidEngineClass::Tracking::getID()
  {
    return _track_id;
  }

  void HidEngineClass::Tracking::onPress(uint8_t n_times)
  {
    HidEngineClass::startTracking(this);
  }

  uint8_t HidEngineClass::Tracking::onRelease()
  {
    HidEngineClass::stopTracking(this);
    return 1;
  }

  //------------------------------------------------------------------+
  // TrackTap
  //------------------------------------------------------------------+
  HidEngineClass::TrackTap::TrackTap(uint8_t track_id, Command *command) : Tracking(track_id), _command(command)
  {
    _command->setParent(this);
  }

  uint8_t HidEngineClass::TrackTap::onRelease()
  {
    Tracking::onRelease();
    if (this->isLastPressed())
    {
      _command->press();
      _command->release();
    }
    return 1;
  }

} // namespace hidpg
