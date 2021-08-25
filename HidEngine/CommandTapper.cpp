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
  namespace Internal
  {

    LinkedList<CommandTapperClass::Info> CommandTapperClass::_list;
    CommandTapperClass::Info CommandTapperClass::_running = {.command = nullptr, .num_of_taps = 0, .tap_speed_ms = 0};
    CommandTapperClass::State CommandTapperClass::_state = CommandTapperClass::State::NotRunning;
    TimerHandle_t CommandTapperClass::_timer_handle = nullptr;
    StaticTimer_t CommandTapperClass::_timer_buffer;

    void CommandTapperClass::begin()
    {
      _timer_handle = xTimerCreateStatic("CommandTapper", 0, false, nullptr, timer_callback, &_timer_buffer);
    }

    bool CommandTapperClass::tap(Command *command, uint8_t n_times, uint16_t tap_speed_ms)
    {
      if (n_times == 0 || command == nullptr)
      {
        return true;
      }

      if (_state == State::NotRunning)
      {
        _state = State::Press;

        _running.command = command;
        _running.num_of_taps = n_times;
        _running.tap_speed_ms = tap_speed_ms;

        _running.command->press(_running.num_of_taps);

        if (pdMS_TO_TICKS(tap_speed_ms) != xTimerGetPeriod(_timer_handle))
        {
          xTimerChangePeriod(_timer_handle, pdMS_TO_TICKS(tap_speed_ms), portMAX_DELAY);
        }
        else
        {
          xTimerStart(_timer_handle, portMAX_DELAY);
        }

        return true;
      }

      if (_running.command == command && _running.tap_speed_ms == tap_speed_ms && _list.size() == 0)
      {
        _running.num_of_taps = constrain(_running.num_of_taps + n_times, 0, UINT8_MAX);

        return true;
      }

      if (_list.size() > 0)
      {
        Info last = _list.get(_list.size() - 1);

        if (last.command == command && last.tap_speed_ms == tap_speed_ms)
        {
          last.num_of_taps = constrain(last.num_of_taps + n_times, 0, UINT8_MAX);
          _list.set(_list.size() - 1, last);

          return true;
        }
      }

      if (_list.size() > HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE)
      {
        return false;
      }

      Info info = {
          .command = command,
          .num_of_taps = n_times,
          .tap_speed_ms = tap_speed_ms,
      };
      _list.add(info);

      return true;
    }

    void CommandTapperClass::onTimer()
    {
      if (_state == State::Press)
      {
        _running.num_of_taps -= _running.command->release();

        if (_running.num_of_taps > 0)
        {
          _state = State::Release;
          xTimerStart(_timer_handle, portMAX_DELAY);
        }
        else if (_list.size() > 0)
        {
          _state = State::ChangeCommandInTheNext;
          xTimerStart(_timer_handle, portMAX_DELAY);
        }
        else
        {
          _state = State::NotRunning;
        }
      }
      else if (_state == State::Release)
      {
        _state = State::Press;
        _running.command->press(_running.num_of_taps);
        xTimerStart(_timer_handle, portMAX_DELAY);
      }
      else if (_state == State::ChangeCommandInTheNext)
      {
        _state = State::Press;

        _running = _list.shift();
        _running.command->press(_running.num_of_taps);

        if (pdMS_TO_TICKS(_running.tap_speed_ms) != xTimerGetPeriod(_timer_handle))
        {
          xTimerChangePeriod(_timer_handle, pdMS_TO_TICKS(_running.tap_speed_ms), portMAX_DELAY);
        }
        else
        {
          xTimerStart(_timer_handle, portMAX_DELAY);
        }
      }
    }

    void CommandTapperClass::timer_callback(TimerHandle_t timerHandle)
    {
      // Software Timersのスタックを消費しないようにstaticで宣言
      static EventData evt;
      evt.event_type = EventType::CommandTapper;
      HidEngineTask.enqueEvent(evt);
    }

  } // namespace Internal

  Internal::CommandTapperClass CommandTapper;

} // namespace hidpg
