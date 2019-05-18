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
#include "HidEngine.h"
#include "Tapper.h"

namespace hidpg
{

TaskHandle_t HidEngineTask::_taskHandle = nullptr;
QueueHandle_t HidEngineTask::_eventQueue = nullptr;

void HidEngineTask::init()
{
  _eventQueue = xQueueCreate(HID_ENGINE_EVENT_QUEUE_SIZE, sizeof(EventData));
}

void HidEngineTask::startTask()
{
  xTaskCreate(task, "HidEngine", HID_ENGINE_TASK_STACK_SIZE, nullptr, TASK_PRIO_LOW, &_taskHandle);
}

void HidEngineTask::sendEventQueue(const EventData &data)
{
  xQueueSend(_eventQueue, &data, portMAX_DELAY);
}

void HidEngineTask::task(void *pvParameters)
{
  while (true)
  {
    EventData data;
    xQueueReceive(_eventQueue, &data, portMAX_DELAY);

    switch (data.eventType)
    {
    case EventType::ApplyToKeymap:
    {
      HidEngine::applyToKeymap_impl(data.applyToKeymap.ids);
      break;
    }
    case EventType::MouseMove:
    {
      HidEngine::mouseMove_impl(data.mouseMove.x, data.mouseMove.y);
      break;
    }
    case EventType::Timer:
    {
      data.timer->cls->trigger(data.timer->number);
      delete data.timer;
      break;
    }
    case EventType::CmdTap:
    {
      CmdTapper::onTimer();
    }
    case EventType::KeyTap:
    {
      KeyTapper::onTimer();
    }
    }
  }
}

} // namespace hidpg
