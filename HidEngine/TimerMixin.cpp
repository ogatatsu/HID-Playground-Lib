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

#include "TimerMixin.h"
#include "HidEngineTask.h"

namespace hidpg
{

void TimerMixin::timeout(TimerHandle_t timerHandle)
{
  // Software Timersのスタックを消費しないようにstaticで宣言
  static EventData edata;
  edata.eventType = EventType::Timer;

  TimerEventData *tdata = static_cast<TimerEventData *>(pvTimerGetTimerID(timerHandle));
  edata.timer = tdata;
  HidEngineTask::enqueEvent(edata);

  xTimerDelete(timerHandle, portMAX_DELAY);
}

TimerMixin::TimerMixin() : _isActive(false), _timerNumber(0)
{
}

void TimerMixin::startTimer(unsigned int ms)
{
  TimerEventData *tdata = new TimerEventData();
  tdata->cls = this;
  tdata->number = ++_timerNumber;

  TimerHandle_t th = xTimerCreate(nullptr, pdMS_TO_TICKS(ms), false, tdata, timeout);
  xTimerStart(th, portMAX_DELAY);

  _isActive = true;
}

void TimerMixin::stopTimer()
{
  _isActive = false;
}

bool TimerMixin::isTimerActive()
{
  return _isActive;
}

// プリエンプティブなマルチタスクだとタスク切換えとstartTimer関数の呼び出しタイミングによっては
// 2個以上のタイマーイベントがイベントキューに入るかもしれないが
// timerNumberを見て最後に作ったイベントのみを実行する仕様とする(使用側からはワンショットタイマーが１個だけ動いてるように見せたい)
void TimerMixin::trigger(unsigned int number)
{
  if (_isActive && (_timerNumber == number))
  {
    _isActive = false;
    onTimer();
  }
}

} // namespace hidpg
