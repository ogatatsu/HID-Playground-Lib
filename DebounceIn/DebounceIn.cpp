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

#include "DebounceIn.h"
#include "DebounceIn_config.h"
#include <Arduino.h>

namespace hidpg
{

DebounceIn::callback_t DebounceIn::_callback = nullptr;
TaskHandle_t DebounceIn::_taskHandle = nullptr;
LinkedList<DebounceIn::PinInfo *> DebounceIn::_list;
uint16_t DebounceIn::_maxDebounceDelay = 0;
uint16_t DebounceIn::_minDebounceDelay = UINT16_MAX;

void DebounceIn::init()
{
  for (int i = 0; i < _list.size(); i++)
  {
    PinInfo *info = _list.get(i);
    info->bounce.attach(info->pin, info->mode);
    attachInterrupt(info->pin, interrupt_callback, CHANGE);
  }
}

void DebounceIn::addPin(uint8_t pin, int mode, uint16_t debounceDelay)
{
  PinInfo *info = new PinInfo;
  info->pin = pin;
  info->mode = mode;
  info->bounce.interval(debounceDelay);
  _list.add(info);

  _maxDebounceDelay = max(debounceDelay, _maxDebounceDelay);
  _minDebounceDelay = min(debounceDelay, _minDebounceDelay);
}

void DebounceIn::startTask()
{
  if (_taskHandle == nullptr)
  {
    xTaskCreate(task, "Debounce", DEBOUNCE_IN_TASK_STACK_SIZE, nullptr, DEBOUNCE_IN_TASK_PRIO, &_taskHandle);
  }
  else
  {
    vTaskResume(_taskHandle);
  }
}

void DebounceIn::stopTask()
{
  if (_taskHandle != nullptr)
  {
    vTaskSuspend(_taskHandle);
  }
}

void DebounceIn::setCallback(callback_t callback)
{
  _callback = callback;
}

void DebounceIn::interrupt_callback()
{
  if (_taskHandle != nullptr)
  {
    xTaskNotifyFromISR(_taskHandle, 2, eSetValueWithOverwrite, nullptr);
  }
}

bool DebounceIn::needsUpdate()
{
  static unsigned long lastInterruptMillis = 0;

  // 割り込みされたら
  if (ulTaskNotifyTake(pdTRUE, 0) > 0)
  {
    lastInterruptMillis = millis();
    return true;
  }
  // 最後に押されてた時間から今の時間までを計算して
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - lastInterruptMillis) <= (_maxDebounceDelay * 3))
  {
    return true;
  }
  return false;
}

void DebounceIn::task(void *pvParameters)
{
  while (true)
  {
    if (needsUpdate())
    {
      // update
      for (int i = 0; i < _list.size(); i++)
      {
        PinInfo *info = _list.get(i);
        if (info->bounce.update())
        {
          if (_callback != nullptr)
          {
            _callback(info->pin, info->bounce.read());
          }
        }
      }
      // poll interval
      delay(_minDebounceDelay);
    }
    else
    {
      // 割り込みが発生するまで寝る
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    }
  }
}

} // namespace hidpg
