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

#include "HidEngineTask.h"
#include "Arduino.h"
#include "CommandTapper.h"
#include "HidEngine.h"

namespace hidpg
{

  TaskHandle_t HidEngineTaskClass::_task_handle = nullptr;
  QueueHandle_t HidEngineTaskClass::_event_queue = nullptr;
  EventData HidEngineTaskClass::_lookahead;

  void HidEngineTaskClass::begin()
  {
    _event_queue = xQueueCreate(HID_ENGINE_EVENT_QUEUE_SIZE, sizeof(EventData));
    xTaskCreate(task, "HidEngine", HID_ENGINE_TASK_STACK_SIZE, nullptr, HID_ENGINE_TASK_PRIO, &_task_handle);
  }

  void HidEngineTaskClass::enqueEvent(const EventData &e_data)
  {
    xQueueSend(_event_queue, &e_data, portMAX_DELAY);
  }

  void HidEngineTaskClass::sumNextMouseMoveEventIfExist(int16_t &x, int16_t &y)
  {
    if (_lookahead.event_type != EventType::Invalid)
    {
      return;
    }

    while (true)
    {
      if (xQueueReceive(_event_queue, &_lookahead, 0) == pdFALSE)
      {
        return;
      }
      if (_lookahead.event_type != EventType::MouseMove)
      {
        return;
      }
      x = constrain(_lookahead.mouse_move.x + x, INT16_MIN, INT16_MAX);
      y = constrain(_lookahead.mouse_move.y + y, INT16_MIN, INT16_MAX);
      _lookahead.event_type = EventType::Invalid;
    }
  }

  void HidEngineTaskClass::task(void *pvParameters)
  {
    while (true)
    {
      EventData buf, *e_data;
      if (_lookahead.event_type != EventType::Invalid)
      {
        e_data = &_lookahead;
      }
      else
      {
        xQueueReceive(_event_queue, &buf, portMAX_DELAY);
        e_data = &buf;
      }

      switch (e_data->event_type)
      {
      case EventType::ApplyToKeymap:
      {
        HidEngine.applyToKeymap_impl(e_data->apply_to_keymap.key_ids);
        break;
      }
      case EventType::TapCommand:
      {
        CommandTapper.tap(e_data->tap_command.command, e_data->tap_command.times);
        break;
      }
      case EventType::MouseMove:
      {
        HidEngine.mouseMove_impl(e_data->mouse_move.x, e_data->mouse_move.y);
        break;
      }
      case EventType::Timer:
      {
        e_data->timer->cls->trigger(e_data->timer->timer_number);
        delete e_data->timer;
        break;
      }
      case EventType::CommandTapper:
      {
        CommandTapper.onTimer();
        break;
      }
      default:
      {
        break;
      }
      }
      e_data->event_type = EventType::Invalid;
    }
  }

  HidEngineTaskClass HidEngineTask;

} // namespace hidpg
