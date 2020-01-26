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

#include "CommandTapper.h"
#include "HidEngineTask.h"
#include "portFreeRTOS.h"
#include <Arduino.h>

namespace hidpg
{

LinkedList<CommandTapper_::Pair> CommandTapper_::_list;
CommandTapper_::Pair CommandTapper_::_running{.command = nullptr, .times = 0};
CommandTapper_::State CommandTapper_::_next_state = CommandTapper_::State::press;
TimerHandle_t CommandTapper_::_timer_handle = nullptr;

void CommandTapper_::init()
{
  _timer_handle = xTimerCreate(nullptr, pdMS_TO_TICKS(1), false, nullptr, timer_callback);
}

void CommandTapper_::tap(Command *command, uint8_t times)
{
  if (times == 0 || command == nullptr)
  {
    return;
  }

  if (_list.size() == 0 && _running.command == nullptr)
  {
    _running.command = command;
    _running.times = times;

    _running.times -= _running.command->press(_running.times);
    _next_state = State::release;
    xTimerStart(_timer_handle, portMAX_DELAY);
    return;
  }

  Pair last = _list.get(_list.size() - 1);
  if (last.command == command)
  {
    last.times += times;
    _list.set(_list.size() - 1, last);
    return;
  }

  last.command = command;
  last.times = times;
  _list.add(last);
}

void CommandTapper_::onTimer()
{
  if (_next_state == State::release)
  {
    _running.command->release();
    _next_state = State::press;
    if (_running.times > 0)
    {
      xTimerStart(_timer_handle, portMAX_DELAY);
    }
    else
    {
      _running.command = nullptr;
      if (_list.size() > 0)
      {
        xTimerStart(_timer_handle, portMAX_DELAY);
      }
    }
  }
  else
  {
    if (_running.command == nullptr)
    {
      _running = _list.shift();
    }
    _running.times -= _running.command->press(_running.times);
    _next_state = State::release;
    xTimerStart(_timer_handle, portMAX_DELAY);
  }
}

void CommandTapper_::timer_callback(TimerHandle_t timerHandle)
{
  // Software Timersのスタックを消費しないようにstaticで宣言
  static EventData e_data;
  e_data.event_type = EventType::CommandTapper;
  HidEngineTask.enqueEvent(e_data);
}

CommandTapper_ CommandTapper;

} // namespace hidpg
