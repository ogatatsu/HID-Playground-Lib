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

using namespace hidpg::Internal;

namespace hidpg
{
  StaticTimer_t TimerMixin::_timer_buffers[HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT];
  TimerMixin::Data TimerMixin::_data_buffers[HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT];
  etl::forward_list<TimerMixin::Data *, HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT> TimerMixin::_pool;

  void TimerMixin::timer_callback(TimerHandle_t timer_handle)
  {
    Data *data = static_cast<TimerMixin::Data *>(pvTimerGetTimerID(timer_handle));

    HidEngineTask.enqueEvent(TimerEventData{data->cls, data->timer_number});

    // プールに戻す
    data->cls = nullptr;
    data->timer_number = 0;
    _pool.push_front(data);
  }

  void TimerMixin::begin()
  {
    for (int i = 0; i < HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT; i++)
    {
      _data_buffers[i].cls = nullptr;
      _data_buffers[i].timer_number = 0;
      _data_buffers[i].timer_handle = xTimerCreateStatic("TimerMixin", 1, pdFALSE, &(_data_buffers[i]), timer_callback, &(_timer_buffers[i]));
      _pool.push_front(&(_data_buffers[i]));
    }
  }

  TimerMixin::TimerMixin() : _is_active(false), _num_of_timer(0)
  {
  }

  bool TimerMixin::startTimer(unsigned int ms)
  {
    if (_pool.empty())
    {
      return false;
    }

    Data *data = _pool.front();
    _pool.pop_front();

    data->cls = this;
    data->timer_number = ++_num_of_timer;

    xTimerChangePeriod(data->timer_handle, pdMS_TO_TICKS(ms), portMAX_DELAY);
    _is_active = true;

    return true;
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
