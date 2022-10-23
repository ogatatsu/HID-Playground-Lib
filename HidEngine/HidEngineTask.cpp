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
#include "CommandTapper.h"
#include "HidEngine.h"

namespace hidpg
{
  namespace Internal
  {

    TaskHandle_t HidEngineTaskClass::_task_handle = nullptr;
    StackType_t HidEngineTaskClass::_task_stack[HID_ENGINE_TASK_STACK_SIZE];
    StaticTask_t HidEngineTaskClass::_task_tcb;
    QueueHandle_t HidEngineTaskClass::_event_queue = nullptr;
    uint8_t HidEngineTaskClass::_event_queue_storage[HID_ENGINE_EVENT_QUEUE_SIZE * sizeof(EventData)];
    StaticQueue_t HidEngineTaskClass::_event_queue_struct;

    void HidEngineTaskClass::start()
    {
      TimerMixin::begin();
      CommandTapper.begin();

      _event_queue = xQueueCreateStatic(HID_ENGINE_EVENT_QUEUE_SIZE, sizeof(EventData), _event_queue_storage, &_event_queue_struct);
      _task_handle = xTaskCreateStatic(task, "HidEngine", HID_ENGINE_TASK_STACK_SIZE, nullptr, HID_ENGINE_TASK_PRIO, _task_stack, &_task_tcb);
    }

    void HidEngineTaskClass::enqueEvent(const EventData &evt)
    {
      xQueueSend(_event_queue, &evt, portMAX_DELAY);
    }

    void HidEngineTaskClass::task(void *pvParameters)
    {
      while (true)
      {
        EventData evt;
        xQueueReceive(_event_queue, &evt, portMAX_DELAY);

        if (auto *e = etl::get_if<ApplyToKeymapEventData>(&evt))
        {
          HidEngine.applyToKeymap_impl(e->key_ids);
        }
        else if (auto *e = etl::get_if<MovePointerEventData>(&evt))
        {
          HidEngine.movePointer_impl(e->pointing_device_id);
        }
        else if (auto *e = etl::get_if<RotateEncoderEventData>(&evt))
        {
          HidEngine.rotateEncoder_impl(e->encoder_id);
        }
        else if (auto *e = etl::get_if<TimerEventData>(&evt))
        {
          e->cls->trigger(e->timer_number);
        }
        else if (etl::holds_alternative<CommandTapperEventData>(evt))
        {
          CommandTapper.onTimer();
        }
      }
    }

    HidEngineTaskClass HidEngineTask;

  } // namespace Internal

} // namespace hidpg
