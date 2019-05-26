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
#include "HidCore.h"
#include "HidEngineTask.h"
#include "Tapper.h"
#include <Arduino.h>

namespace hidpg
{

/*------------------------------------------------------------------*/
/* SequenceMode
 *------------------------------------------------------------------*/
uint8_t SequenceMode::onPress(uint8_t accrued)
{
  HidEngine::switchSequenceMode();
  return 1;
}

/*------------------------------------------------------------------*/
/* Tracking
 *------------------------------------------------------------------*/
Tracking::Tracking(uint8_t id) : _id(id)
{
}

uint8_t Tracking::getID()
{
  return _id;
}

uint8_t Tracking::onPress(uint8_t accrued)
{
  HidEngine::startTracking(this);
  return 1;
}

void Tracking::onRelease()
{
  HidEngine::stopTracking(this);
}

/*------------------------------------------------------------------*/
/* TrackTap
 *------------------------------------------------------------------*/
TrackTap::TrackTap(uint8_t id, Command *command) : Tracking(id), _command(command)
{
  _command->setParent(this);
}

uint8_t TrackTap::onPress(uint8_t accrued)
{
  return Tracking::onPress(accrued);
}

void TrackTap::onRelease()
{
  Tracking::onRelease();
  if (this->isLastPressed())
  {
    _command->press();
    _command->release();
  }
}

/*------------------------------------------------------------------*/
/* HidEngine
 *------------------------------------------------------------------*/
ID_and_Command *HidEngine::_keymap = nullptr;
SimulIDs_and_Command *HidEngine::_simulKeymap = nullptr;
SeqIDs_and_Command *HidEngine::_seqKeymap = nullptr;
TrackID_and_Command *HidEngine::_trackmap = nullptr;
uint8_t HidEngine::_keymapLength = 0;
uint8_t HidEngine::_simulKeymapLength = 0;
uint8_t HidEngine::_seqKeymapLength = 0;
uint8_t HidEngine::_trackmapLength = 0;

HidEngine::SeqModeState HidEngine::_seqModeState = HidEngine::SeqModeState::Disable;
LinkedList<Tracking *> HidEngine::_trackingList;
int HidEngine::_distanceX = 0;
int HidEngine::_distanceY = 0;

void HidEngine::init(HidReporter *hidReporter)
{
  Hid::setReporter(hidReporter);
  HidEngineTask::init();
  CmdTapper::init();
}

void HidEngine::startTask()
{
  HidEngineTask::startTask();
}

void HidEngine::applyToKeymap(const Set &ids)
{
  EventData data;
  data.eventType = EventType::ApplyToKeymap;
  data.applyToKeymap.ids = ids;
  HidEngineTask::sendEventQueue(data);
}

void HidEngine::mouseMove(int8_t x, int8_t y)
{
  EventData data;
  data.eventType = EventType::MouseMove;
  data.mouseMove.x = x;
  data.mouseMove.y = y;
  HidEngineTask::sendEventQueue(data);
}

size_t HidEngine::getValidLength(const uint8_t ids[], size_t maxLength)
{
  size_t i = 0;
  for (; i < maxLength; i++)
  {
    if (ids[i] == 0)
    {
      break;
    }
  }
  return i;
}

void HidEngine::applyToKeymap_impl(const Set &ids)
{
  static Set prevIDs, pressedInSeqModeIDs;
  static uint8_t idSeq[MAX_SEQ_COUNT];
  static size_t idSeqLen = 0;
  static SeqIDs_and_Command *matched;

  // シーケンスモード中に押されたIDがリリースされるまで監視する
  if (pressedInSeqModeIDs.count() != 0)
  {
    // １つ前のIDs - 現在のIDs = リリースされたIDs
    Set releaseIDs = prevIDs - ids;
    pressedInSeqModeIDs -= releaseIDs;
  }

  // apply to normal keymap
  for (size_t i = 0; i < _keymapLength; i++)
  {
    // シーケンスモード中に押されたIDなら何もしない
    if (pressedInSeqModeIDs.contains(_keymap[i].id))
    {
      continue;
    }
    // IDが押されているかを取得
    bool pressed = ids.contains(_keymap[i].id);
    // シーケンスモード中はkeymapのコマンドは新しく実行しない、リリースのみ許可する
    if ((pressed == true) && (_seqModeState == SeqModeState::Running))
    {
      continue;
    }
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

  // apply to simul keymap
  for (size_t i = 0; i < _simulKeymapLength; i++)
  {
    // シーケンスモード中に押されたIDが含まれていたら何もしない
    if (pressedInSeqModeIDs.containsAny(_simulKeymap[i].ids, _simulKeymap[i].idsLength))
    {
      continue;
    }
    // 全てのIDが押されているかを取得
    bool pressed = ids.containsAll(_simulKeymap[i].ids, _simulKeymap[i].idsLength);
    // シーケンスモード中は時はsimulKeymapのコマンドは新しく実行しない、リリースのみ許可する
    if ((pressed == true) && (_seqModeState == SeqModeState::Running))
    {
      continue;
    }
    // コマンドに現在の状態を適用する
    if (pressed)
    {
      _simulKeymap[i].command->press();
    }
    else
    {
      _simulKeymap[i].command->release();
    }
  }

  // apply to seq keymap
  if (_seqModeState == SeqModeState::Triggered)
  {
    // SEQ_MODEコマンドが実行されたらseqModeStateがTriggeredになりここに来る
    // 次の入力からシーケンスモードを開始する
    _seqModeState = SeqModeState::Running;
  }
  else if (_seqModeState == SeqModeState::Running)
  {
    // 新しく押されたIDを順番に保存していきseqKeymap内で一致している物があるかどうかを調べる
    // 現在のIDs - １つ前のIDs = 新しく押されたIDs
    Set newPressIDs = ids - prevIDs;
    // Setだと直接値を取得できないので配列に変換
    uint16_t len = newPressIDs.count();
    uint8_t buf[len];
    newPressIDs.toArray(buf);
    // idSeqにシーケンスモードになってから新しく押されたIDを順番に保存する
    size_t i = 0;
    while ((idSeqLen < MAX_SEQ_COUNT) && (i < len))
    {
      idSeq[idSeqLen++] = buf[i++];
    }
    // 順番に保存したIDとseqKeymapの比較
    int matchResult = match_with_seqKeymap(idSeq, idSeqLen, &matched);
    if (matchResult == 0)
    { // マッチしなければシーケンスモードを解除
      idSeqLen = 0;
      _seqModeState = SeqModeState::Disable;
    }
    else if (matchResult == 1)
    { // 部分マッチならば何もしない
      // pass
    }
    else if (matchResult == 2)
    { // 完全マッチしたらコマンドを実行してStateをWaitReleaseに移行
      idSeqLen = 0;
      matched->command->press();
      _seqModeState = SeqModeState::WaitRelease;
    }

    // シーケンスモード時に押されたIDは1回リリースされるまではkeymap,simulKeymapのコマンドを実行しない
    // そのためリリースされるまで監視する必要があるのでIDを追加していく
    pressedInSeqModeIDs |= newPressIDs;
  }
  else if (_seqModeState == SeqModeState::WaitRelease)
  {
    // 完全マッチ時に実行したコマンドを解除
    // ids内からマッチしたID列の最後のID無くなったら解除する
    if (ids.contains(matched->ids[matched->idsLength - 1]) == false)
    {
      matched->command->release();
      _seqModeState = SeqModeState::Disable;
    }
  }

  // 1つ前のIDとして保存
  prevIDs = ids;
}

// 引数のidsがseqKeymapの定義とマッチするか調べる
// 戻り値が
// 0: マッチしない
// 1: 部分マッチ
// 2: 完全にマッチ、完全にマッチした場合はmatchedにマッチしたSeqIDs_and_Commandを入れて返す
int HidEngine::match_with_seqKeymap(const uint8_t ids[], size_t len, SeqIDs_and_Command **matched)
{
  for (size_t i = 0; i < _seqKeymapLength; i++)
  {
    size_t minLen = min(len, _seqKeymap[i].idsLength);
    if (memcmp(ids, _seqKeymap[i].ids, minLen) == 0)
    {
      if (len == _seqKeymap[i].idsLength)
      {
        *matched = &_seqKeymap[i];
        return 2;
      }
      return 1;
    }
  }
  return 0;
}

void HidEngine::mouseMove_impl(int8_t x, int8_t y)
{
  static uint8_t beforeID;

  if (_trackingList.size() > 0)
  {
    // 一番上のtrackIDを取得
    uint8_t trackID = _trackingList[0]->getID();
    // 前回のIDと違うなら0から距離を測る
    if (trackID != beforeID)
    {
      _distanceX = _distanceY = 0;
      beforeID = trackID;
    }
    // 距離を足す
    _distanceX += x;
    _distanceY += y;
    // trackmapから一致するtrackIDのインデックスを検索
    int idx = -1;
    for (int i = 0; i < _trackmapLength; i++)
    {
      if (_trackmap[i].trackID == trackID)
      {
        idx = i;
        break;
      }
    }
    // 一致するtrackIDが無かったら抜ける
    if (idx == -1)
    {
      return;
    }
    // 測った距離が閾値を超えてたらコマンドを発火する
    int16_t threshold = _trackmap[idx].distance;
    if (_distanceY <= -threshold)
    {
      uint8_t times = min(abs(_distanceY / threshold), UINT8_MAX);
      _distanceY %= threshold;
      if (_trackmap[idx].upCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].upCommand, times);
      }
    }
    else if (_distanceY >= threshold)
    {
      uint8_t times = min(_distanceY / threshold, UINT8_MAX);
      _distanceY %= threshold;
      if (_trackmap[idx].downCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].downCommand, times);
      }
    }

    if (_distanceX <= -threshold)
    {
      uint8_t times = min(abs(_distanceX / threshold), UINT8_MAX);
      _distanceX %= threshold;
      if (_trackmap[idx].leftCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].leftCommand, times);
      }
    }
    else if (_distanceX >= threshold)
    {
      uint8_t times = min(_distanceX / threshold, UINT8_MAX);
      _distanceX %= threshold;

      if (_trackmap[idx].rightCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].rightCommand, times);
      }
    }
  }
  else
  {
    Hid::mouseMove(x, y);
  }
}

void HidEngine::switchSequenceMode()
{
  if (_seqModeState == SeqModeState::Disable)
  {
    _seqModeState = SeqModeState::Triggered;
  }
}

void HidEngine::startTracking(Tracking *tracking)
{
  _trackingList.unshift(tracking);
}

void HidEngine::stopTracking(Tracking *tracking)
{
  for (int i = 0; i < _trackingList.size(); i++)
  {
    if (_trackingList.get(i) == tracking)
    {
      _trackingList.remove(i);
      if (_trackingList.size() == 0)
      {
        _distanceX = _distanceY = 0;
      }
      return;
    }
  }
}

} // namespace hidpg
