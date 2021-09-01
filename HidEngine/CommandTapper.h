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

#include "Command.h"
#include "etl/deque.h"

namespace hidpg
{
  namespace Internal
  {

    class CommandTapperClass
    {
      friend class HidEngineTaskClass;

    public:
      static void begin();
      static bool tap(Command *command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS);

    private:
      static void onTimer();
      static void timer_callback(TimerHandle_t timer_handle);

      struct Info
      {
        Command *command;
        uint8_t num_of_taps;
        uint16_t tap_speed_ms;
      };

      enum class State
      {
        NotRunning,
        Press,
        Release,
        ChangeCommandInTheNext,
      };

      static etl::deque<Info, HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE> _deque;
      static Info _running;
      static State _state;
      static TimerHandle_t _timer_handle;
      static StaticTimer_t _timer_buffer;
    };

  } // namespace Internal

  extern Internal::CommandTapperClass CommandTapper;

} // namespace hidpg
