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
#include "Arduino.h"
#include "FreeRTOS.h"
#include "HidEngineTask.h"

namespace hidpg
{

  LinkedList<CommandTapperClass::Pair> CommandTapperClass::_list;
  CommandTapperClass::Pair CommandTapperClass::_running{.command = nullptr, .num_of_taps = 0};
  CommandTapperClass::State CommandTapperClass::_next_state = CommandTapperClass::State::press;
  TimerHandle_t CommandTapperClass::_timer_handle = nullptr;

  void CommandTapperClass::begin()
  {
    _timer_handle = xTimerCreate(nullptr, pdMS_TO_TICKS(HID_ENGINE_TAP_SPEED_MS), false, nullptr, timer_callback);
  }

  bool CommandTapperClass::tap(Command *command, uint8_t n_times)
  {
    if (n_times == 0 || command == nullptr)
    {
      return true;
    }

    if (_list.size() == 0 && _running.command == nullptr)
    {
      _running.command = command;
      _running.num_of_taps = n_times;

      _running.command->press(_running.num_of_taps);
      _next_state = State::release;
      xTimerStart(_timer_handle, portMAX_DELAY);
      return true;
    }

    Pair last = _list.get(_list.size() - 1);
    if (last.command == command)
    {
      last.num_of_taps = constrain(last.num_of_taps + n_times, 0, UINT8_MAX);
      _list.set(_list.size() - 1, last);
      return true;
    }

    if (_list.size() > HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE)
    {
      return false;
    }

    last.command = command;
    last.num_of_taps = n_times;
    _list.add(last);
    return true;
  }

  void CommandTapperClass::onTimer()
  {
    if (_next_state == State::release)
    {
      _running.num_of_taps -= _running.command->release();
      _next_state = State::press;
      if (_running.num_of_taps > 0)
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
      _running.command->press(_running.num_of_taps);
      _next_state = State::release;
      xTimerStart(_timer_handle, portMAX_DELAY);
    }
  }

  void CommandTapperClass::timer_callback(TimerHandle_t timerHandle)
  {
    // Software Timersのスタックを消費しないようにstaticで宣言
    static EventData evt;
    evt.event_type = EventType::CommandTapper;
    HidEngineTask.enqueEvent(evt);
  }

  CommandTapperClass CommandTapper;

} // namespace hidpg
