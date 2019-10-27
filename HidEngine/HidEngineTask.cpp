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
#include "CmdTapper.h"
#include "HidEngine.h"
#include <Arduino.h>

namespace hidpg
{

TaskHandle_t HidEngineTask::_taskHandle = nullptr;
QueueHandle_t HidEngineTask::_eventQueue = nullptr;
EventData HidEngineTask::_lookAhead;

void HidEngineTask::init()
{
  _eventQueue = xQueueCreate(HID_ENGINE_EVENT_QUEUE_SIZE, sizeof(EventData));
}

void HidEngineTask::startTask()
{
  xTaskCreate(task, "HidEngine", HID_ENGINE_TASK_STACK_SIZE, nullptr, TASK_PRIO_LOW, &_taskHandle);
}

void HidEngineTask::enqueEvent(const EventData &data)
{
  xQueueSend(_eventQueue, &data, portMAX_DELAY);
}

void HidEngineTask::sumNextMouseMoveEventIfExist(int16_t &x, int16_t &y)
{
  if (_lookAhead.eventType != EventType::Invalid)
  {
    return;
  }

  while (true)
  {
    if (xQueueReceive(_eventQueue, &_lookAhead, 0) == pdFALSE)
    {
      return;
    }
    if (_lookAhead.eventType != EventType::MouseMove)
    {
      return;
    }
    x = constrain(_lookAhead.mouseMove.x + x, INT16_MIN, INT16_MAX);
    y = constrain(_lookAhead.mouseMove.y + y, INT16_MIN, INT16_MAX);
    _lookAhead.eventType = EventType::Invalid;
  }
}

void HidEngineTask::task(void *pvParameters)
{
  while (true)
  {
    EventData buf, *edata;
    if (_lookAhead.eventType != EventType::Invalid)
    {
      edata = &_lookAhead;
    }
    else
    {
      xQueueReceive(_eventQueue, &buf, portMAX_DELAY);
      edata = &buf;
    }

    switch (edata->eventType)
    {
    case EventType::ApplyToKeymap:
    {
      HidEngine::applyToKeymap_impl(edata->applyToKeymap.keyIDs);
      break;
    }
    case EventType::TapCommand:
    {
      CmdTapper::tap(edata->tapCommand.command, edata->tapCommand.times);
      break;
    }
    case EventType::MouseMove:
    {
      HidEngine::mouseMove_impl(edata->mouseMove.x, edata->mouseMove.y);
      break;
    }
    case EventType::Timer:
    {
      edata->timer->cls->trigger(edata->timer->number);
      delete edata->timer;
      break;
    }
    case EventType::CmdTapper:
    {
      CmdTapper::onTimer();
      break;
    }
    default:
    {
      break;
    }
    }
    edata->eventType = EventType::Invalid;
  }
}

} // namespace hidpg
