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

  void TimerMixin::timer_callback(TimerHandle_t timer_handle)
  {
    // Software Timersのスタックを消費しないようにstaticで宣言
    static EventData evt;
    evt.event_type = EventType::Timer;

    TimerEventData *te_data = static_cast<TimerEventData *>(pvTimerGetTimerID(timer_handle));
    evt.timer = te_data;
    HidEngineTask.enqueEvent(evt);

    xTimerDelete(timer_handle, portMAX_DELAY);
  }

  TimerMixin::TimerMixin() : _is_active(false), _num_of_timer(0)
  {
  }

  void TimerMixin::startTimer(unsigned int ms)
  {
    TimerEventData *te_data = new TimerEventData();
    te_data->cls = this;
    te_data->timer_number = ++_num_of_timer;

    TimerHandle_t th = xTimerCreate(nullptr, pdMS_TO_TICKS(ms), false, te_data, timer_callback);
    xTimerStart(th, portMAX_DELAY);

    _is_active = true;
  }

  void TimerMixin::stopTimer()
  {
    _is_active = false;
  }

  bool TimerMixin::isTimerActive()
  {
    return _is_active;
  }

  // プリエンプティブなマルチタスクだとタスク切換えとstartTimer関数の呼び出しタイミングによっては
  // 2個以上のタイマーイベントがイベントキューに入るかもしれないが
  // timer_numberを見て最後に作ったイベントのみを実行する仕様とする(使用側からはワンショットタイマーが１個だけ動いてるように見える)
  void TimerMixin::trigger(unsigned int timer_number)
  {
    if (_is_active && (_num_of_timer == timer_number))
    {
      _is_active = false;
      onTimer();
    }
  }

} // namespace hidpg
