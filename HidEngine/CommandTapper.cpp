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

namespace hidpg
{
  namespace Internal
  {

    etl::list<CommandTapperClass::Data, HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE> CommandTapperClass::_queue;
    CommandTapperClass::Data CommandTapperClass::_running = {.command = nullptr, .num_of_taps = 0, .tap_speed_ms = 0};
    CommandTapperClass::State CommandTapperClass::_state = CommandTapperClass::State::NotRunning;
    TimerHandle_t CommandTapperClass::_timer_handle = nullptr;
    StaticTimer_t CommandTapperClass::_timer_buffer;

    void CommandTapperClass::begin()
    {
      _timer_handle = xTimerCreateStatic("CommandTapper", 1, pdFALSE, nullptr, timer_callback, &_timer_buffer);
    }

    bool CommandTapperClass::tap(Command *command, uint8_t n_times, uint16_t tap_speed_ms)
    {
      if (n_times == 0 || command == nullptr)
      {
        return true;
      }

      // 動いてないならタップしてタップ回数が残ってるならタイマーを動かす
      if (_state == State::NotRunning)
      {
        uint8_t actual_n_times = command->tap(n_times);
        uint8_t num_of_taps = std::max(n_times - actual_n_times, 0);

        if (num_of_taps != 0)
        {
          _state = State::WaitTimer;
          _running.command = command;
          _running.num_of_taps = num_of_taps;
          _running.tap_speed_ms = tap_speed_ms;
          xTimerChangePeriod(_timer_handle, pdMS_TO_TICKS(tap_speed_ms), portMAX_DELAY);
        }

        return true;
      }

      // キューが空で今動いてるコマンドと同じならタップ回数を足す
      if (_queue.empty() && _running.command == command && _running.tap_speed_ms == tap_speed_ms)
      {
        _running.num_of_taps = std::min(_running.num_of_taps + n_times, UINT8_MAX);
        return true;
      }

      // キューが空でないならキューの最後のコマンドと比較して同じならタップ回数を足す
      if (_queue.empty() == false)
      {
        Data &last = _queue.back();

        if (last.command == command && last.tap_speed_ms == tap_speed_ms)
        {
          last.num_of_taps = std::min(last.num_of_taps + n_times, UINT8_MAX);
          return true;
        }
      }

      // キューに空きがあるなら追加
      if (_queue.available())
      {
        Data data = {
            .command = command,
            .num_of_taps = n_times,
            .tap_speed_ms = tap_speed_ms,
        };
        _queue.push_back(data);
        return true;
      }

      return false;
    }

    void CommandTapperClass::onTimer()
    {
      uint8_t actual_n_times = _running.command->tap(_running.num_of_taps);
      _running.num_of_taps = std::max(_running.num_of_taps - actual_n_times, 0);

      // まだタップ回数が残っている場合同じコマンドで再度タップ
      if (_running.num_of_taps > 0)
      {
        _state = State::WaitTimer;
        xTimerStart(_timer_handle, portMAX_DELAY);
      }
      // キューが空でないなら次のコマンドの準備
      else if (_queue.empty() == false)
      {
        _running = _queue.front();
        _queue.pop_front();
        xTimerChangePeriod(_timer_handle, pdMS_TO_TICKS(_running.tap_speed_ms), portMAX_DELAY);
      }
      // 動作終了
      else
      {
        _state = State::NotRunning;
        _running.command = nullptr;
        _running.num_of_taps = 0;
        _running.tap_speed_ms = 0;
      }
    }

    void CommandTapperClass::timer_callback(TimerHandle_t timerHandle)
    {
      HidEngineTask.enqueEvent(CommandTapperEventData{});
    }

  } // namespace Internal

  Internal::CommandTapperClass CommandTapper;

} // namespace hidpg
