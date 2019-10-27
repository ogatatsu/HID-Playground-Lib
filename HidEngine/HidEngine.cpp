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
#include "CmdTapper.h"
#include "HidCore.h"
#include "HidEngineTask.h"
#include <Arduino.h>

namespace hidpg
{
/*------------------------------------------------------------------*/
/* HidEngine
 *------------------------------------------------------------------*/
Key *HidEngine::_keymap = nullptr;
SimulKey *HidEngine::_simulKeymap = nullptr;
SeqKey *HidEngine::_seqKeymap = nullptr;
Track *HidEngine::_trackmap = nullptr;
uint8_t HidEngine::_keymapLength = 0;
uint8_t HidEngine::_simulKeymapLength = 0;
uint8_t HidEngine::_seqKeymapLength = 0;
uint8_t HidEngine::_trackmapLength = 0;

HidEngine::SeqModeState HidEngine::_seqModeState = HidEngine::SeqModeState::Disable;
LinkedList<HidEngine::Tracking *> HidEngine::_trackingList;
int32_t HidEngine::_distanceX = 0;
int32_t HidEngine::_distanceY = 0;

void HidEngine::setHidReporter(HidReporter *hidReporter)
{
  Hid::setReporter(hidReporter);
}

void HidEngine::init()
{
  HidEngineTask::init();
  CmdTapper::init();
}

void HidEngine::startTask()
{
  HidEngineTask::startTask();
}

void HidEngine::applyToKeymap(const Set &keyIDs)
{
  EventData data;
  data.eventType = EventType::ApplyToKeymap;
  data.applyToKeymap.keyIDs = keyIDs;
  HidEngineTask::enqueEvent(data);
}

void HidEngine::tapCommand(Command *command, uint8_t times)
{
  EventData data;
  data.eventType = EventType::TapCommand;
  data.tapCommand.command = command;
  data.tapCommand.times = times;
  HidEngineTask::enqueEvent(data);
}

void HidEngine::mouseMove(int16_t x, int16_t y)
{
  EventData data;
  data.eventType = EventType::MouseMove;
  data.mouseMove.x = x;
  data.mouseMove.y = y;
  HidEngineTask::enqueEvent(data);
}

size_t HidEngine::getValidLength(const uint8_t keyIDs[], size_t maxLength)
{
  size_t i = 0;
  for (; i < maxLength; i++)
  {
    if (keyIDs[i] == 0)
    {
      break;
    }
  }
  return i;
}

void HidEngine::applyToKeymap_impl(const Set &keyIDs)
{
  static Set prevIDs, pressedInSeqModeIDs;
  static uint8_t idSeq[MAX_SEQ_COUNT];
  static size_t idSeqLen = 0;
  static SeqKey *matched;

  // シーケンスモード中に押されたIDがリリースされるまで監視する
  if (pressedInSeqModeIDs.count() != 0)
  {
    // 1つ前のIDs - 現在のIDs = リリースされたIDs
    Set releaseIDs = prevIDs - keyIDs;
    pressedInSeqModeIDs -= releaseIDs;
  }

  // apply to keymap
  for (size_t i = 0; i < _keymapLength; i++)
  {
    // シーケンスモード中に押されたIDなら何もしない
    if (pressedInSeqModeIDs.contains(_keymap[i].keyID))
    {
      continue;
    }
    // IDが押されているかを取得
    bool pressed = keyIDs.contains(_keymap[i].keyID);
    // シーケンスモード中はコマンドを新しく実行しない、リリースのみ許可する
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

  // apply to simulKeymap
  for (size_t i = 0; i < _simulKeymapLength; i++)
  {
    // シーケンスモード中に押されたIDが含まれていたら何もしない
    if (pressedInSeqModeIDs.containsAny(_simulKeymap[i].keyIDs, _simulKeymap[i].idsLength))
    {
      continue;
    }
    // 全てのIDが押されているかを取得
    bool pressed = keyIDs.containsAll(_simulKeymap[i].keyIDs, _simulKeymap[i].idsLength);
    // シーケンスモード中はコマンドを新しく実行しない、リリースのみ許可する
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

  // apply to seqKeymap
  if (_seqModeState == SeqModeState::Triggered)
  {
    // SEQ_MODEコマンドが実行されたらseqModeStateがTriggeredになりここに来る
    // 次の入力からシーケンスモードを開始する
    _seqModeState = SeqModeState::Running;
  }
  else if (_seqModeState == SeqModeState::Running)
  {
    // 新しく押されたIDを順番に保存していきseqKeymap内で一致している物があるかどうかを調べる
    // 現在のIDs - 1つ前のIDs = 新しく押されたIDs
    Set newPressIDs = keyIDs - prevIDs;
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
    // keyIDs内からマッチしたID列の最後のID無くなったら解除する
    if (keyIDs.contains(matched->keyIDs[matched->idsLength - 1]) == false)
    {
      matched->command->release();
      _seqModeState = SeqModeState::Disable;
    }
  }

  // 1つ前のIDとして保存
  prevIDs = keyIDs;
}

// 引数のidSeqがseqKeymapの定義とマッチするか調べる
// 戻り値が
// 0: マッチしない
// 1: 部分マッチ
// 2: 完全にマッチ、完全にマッチした場合はmatchedにマッチしたSeqKeyを入れて返す
int HidEngine::match_with_seqKeymap(const uint8_t idSeq[], size_t len, SeqKey **matched)
{
  for (size_t i = 0; i < _seqKeymapLength; i++)
  {
    size_t minLen = min(len, _seqKeymap[i].idsLength);
    if (memcmp(idSeq, _seqKeymap[i].keyIDs, minLen) == 0)
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

void HidEngine::mouseMove_impl(int16_t x, int16_t y)
{
  static uint8_t beforeID;

  if (_trackingList.size() > 0) // Tracking Commandが実行中の場合
  {
    // 一番上のtrackIDを取得
    uint8_t trackID = _trackingList[0]->getID();
    // 前回のIDと違うなら0から距離を測る
    if (trackID != beforeID)
    {
      _distanceX = _distanceY = 0;
      beforeID = trackID;
    }
    // 効率のため、次のイベントがMouseMoveだったら合計してまとめて行う
    HidEngineTask::sumNextMouseMoveEventIfExist(x, y);
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
    if (_distanceY <= -threshold) // 上
    {
      uint8_t times = min(static_cast<int>(abs(_distanceY / threshold)), UINT8_MAX);
      _distanceY %= threshold;
      if (_trackmap[idx].upCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].upCommand, times);
      }
    }
    else if (_distanceY >= threshold) // 下
    {
      uint8_t times = min(static_cast<int>(_distanceY / threshold), UINT8_MAX);
      _distanceY %= threshold;
      if (_trackmap[idx].downCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].downCommand, times);
      }
    }

    if (_distanceX <= -threshold) // 左
    {
      uint8_t times = min(static_cast<int>(abs(_distanceX / threshold)), UINT8_MAX);
      _distanceX %= threshold;
      if (_trackmap[idx].leftCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].leftCommand, times);
      }
    }
    else if (_distanceX >= threshold) // 右
    {
      uint8_t times = min(static_cast<int>(_distanceX / threshold), UINT8_MAX);
      _distanceX %= threshold;

      if (_trackmap[idx].rightCommand != nullptr)
      {
        CmdTapper::tap(_trackmap[idx].rightCommand, times);
      }
    }
  }
  else
  {
    while (!(x == 0 && y == 0))
    {
      // 効率のため、次のイベントがMouseMoveだったら合計してまとめて行う
      HidEngineTask::sumNextMouseMoveEventIfExist(x, y);
      // １回で動かせる量は -127 ~ 127
      int8_t deltaX = constrain(x, -127, 127);
      x -= deltaX;
      int8_t deltaY = constrain(y, -127, 127);
      y -= deltaY;
      Hid::mouseMove(deltaX, deltaY);
    }
  }
}

void HidEngine::switchSequenceMode()
{
  if (_seqModeState == SeqModeState::Disable)
  {
    _seqModeState = SeqModeState::Triggered;
  }
}

void HidEngine::startTracking(HidEngine::Tracking *tracking)
{
  _trackingList.unshift(tracking);
}

void HidEngine::stopTracking(HidEngine::Tracking *tracking)
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

/*------------------------------------------------------------------*/
/* SequenceMode
 *------------------------------------------------------------------*/
uint8_t HidEngine::SequenceMode::onPress(uint8_t accrued)
{
  HidEngine::switchSequenceMode();
  return 1;
}

/*------------------------------------------------------------------*/
/* Tracking
 *------------------------------------------------------------------*/
HidEngine::Tracking::Tracking(uint8_t trackID) : _trackID(trackID)
{
}

uint8_t HidEngine::Tracking::getID()
{
  return _trackID;
}

uint8_t HidEngine::Tracking::onPress(uint8_t accrued)
{
  HidEngine::startTracking(this);
  return 1;
}

void HidEngine::Tracking::onRelease()
{
  HidEngine::stopTracking(this);
}

/*------------------------------------------------------------------*/
/* TrackTap
 *------------------------------------------------------------------*/
HidEngine::TrackTap::TrackTap(uint8_t trackID, Command *command) : Tracking(trackID), _command(command)
{
  _command->setParent(this);
}

void HidEngine::TrackTap::onRelease()
{
  Tracking::onRelease();
  if (this->isLastPressed())
  {
    _command->press();
    _command->release();
  }
}

} // namespace hidpg
