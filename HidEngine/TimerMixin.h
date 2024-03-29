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

#pragma once

#include "FreeRTOS.h"
#include "HidEngine_config.h"
#include "etl/forward_list.h"
#include "timers.h"

namespace hidpg
{
  namespace Internal
  {
    class HidEngineTaskClass;
  }

  class TimerMixin
  {
    friend class Internal::HidEngineTaskClass;

  protected:
    TimerMixin();
    bool startTimer(unsigned int ms);
    void stopTimer();
    bool isTimerActive();
    virtual void onTimer() {}

  private:
    struct Data
    {
      TimerMixin *cls;
      unsigned int timer_number;
      TimerHandle_t timer_handle;
    };

    void trigger(unsigned int timer_number);

    static void begin();
    static void timer_callback(TimerHandle_t timer_handle);

    bool _is_active;
    unsigned int _num_of_timer;

    static StaticTimer_t _timer_buffers[HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT];
    static Data _data_buffers[HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT];
    static etl::forward_list<Data *, HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT> _pool;
  };

} // namespace hidpg
