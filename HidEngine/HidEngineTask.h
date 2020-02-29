
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

#include "Command.h"
#include "Set.h"
#include "TimerMixin.h"
#include "queue.h"

namespace hidpg
{

enum class EventType
{
  Invalid,
  ApplyToKeymap,
  TapCommand,
  MouseMove,
  Timer,
  CommandTapper,
};

struct ApplyToKeymapEventData
{
  Set key_ids;
};

struct TapCommandEventData
{
  Command *command;
  uint8_t times;
};

struct MouseMoveEventData
{
  int16_t x;
  int16_t y;
};

struct TimerEventData
{
  TimerMixin *cls;
  unsigned int timer_number;
};

struct CommandTapperEventData
{
  //empty
};

struct EventData
{
  EventData() : event_type(EventType::Invalid){};

  EventType event_type;
  union {
    ApplyToKeymapEventData apply_to_keymap;
    TapCommandEventData tap_command;
    MouseMoveEventData mouse_move;
    TimerEventData *timer;
    CommandTapperEventData command_tapper;
  };
};

class HidEngineTask_
{
public:
  static void init();
  static void startTask();
  static void enqueEvent(const EventData &e_data);
  static void sumNextMouseMoveEventIfExist(int16_t &x, int16_t &y);

private:
  static void task(void *pvParameters);

  static TaskHandle_t _task_handle;
  static QueueHandle_t _event_queue;
  static EventData _lookahead;
};

extern HidEngineTask_ HidEngineTask;

} // namespace hidpg
