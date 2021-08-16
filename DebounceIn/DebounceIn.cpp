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
#include "Arduino.h"
#include "SenseInterrupt.h"

#define pdMS_TO_TICKS_DOUBLE(xTimeInMs) ((double)(((double)(xTimeInMs) * (double)configTICK_RATE_HZ) / (double)1000))

namespace hidpg
{

  DebounceInClass::callback_t DebounceInClass::_callback = nullptr;

  Bounce DebounceInClass::_bounce_list[DEBOUNCE_IN_MAX_PIN_COUNT];
  uint8_t DebounceInClass::_bounce_list_len = 0;

  uint16_t DebounceInClass::_max_debounce_delay_ms = 0;
  uint16_t DebounceInClass::_polling_interval_ms = UINT16_MAX;
  uint16_t DebounceInClass::_max_polling_count = 0;

  TaskHandle_t DebounceInClass::_task_handle = nullptr;
  StackType_t DebounceInClass::_task_stack[DEBOUNCE_IN_TASK_STACK_SIZE];
  StaticTask_t DebounceInClass::_task_tcb;

  void DebounceInClass::start()
  {

#if DEBOUNCE_IN_USE_SENSE_INTERRUPT == true
    attachSenseInterrupt(interrupt_callback);
#endif

    // _polling_interval_ms * _max_polling_countは最低でもdebounce_delayの最大値を超える値に設定
    // delay関数で切り捨てられるtick回数も考慮して計算
    _max_polling_count = ceil(pdMS_TO_TICKS_DOUBLE(_max_debounce_delay_ms) / pdMS_TO_TICKS(_polling_interval_ms)) + 3;

    _task_handle = xTaskCreateStatic(task, "DebounceIn", DEBOUNCE_IN_TASK_STACK_SIZE, nullptr, DEBOUNCE_IN_TASK_PRIO, _task_stack, &_task_tcb);
  }

  bool DebounceInClass::addPin(uint8_t pin, int mode, uint16_t debounce_delay_ms)
  {
#if DEBOUNCE_IN_USE_SENSE_INTERRUPT == false
    if (_bounce_list_len >= DEBOUNCE_IN_MAX_PIN_COUNT)
    {
      return false;
    }
#endif

    _bounce_list[_bounce_list_len].attach(pin, mode);
    _bounce_list[_bounce_list_len].interval(debounce_delay_ms);
    _bounce_list_len++;

#if DEBOUNCE_IN_USE_SENSE_INTERRUPT == false
    attachInterrupt(pin, interrupt_callback, CHANGE);
#endif

    // 後でポーリング回数を決めるためにdebounce_delayの最大値を保存しておく
    _max_debounce_delay_ms = max(debounce_delay_ms, _max_debounce_delay_ms);
    // debounce_delayの最小値の間隔でポーリングする
    _polling_interval_ms = min(debounce_delay_ms, _polling_interval_ms);

    return true;
  }

  void DebounceInClass::stopTask()
  {
    if (_task_handle != nullptr)
    {
      vTaskSuspend(_task_handle);
    }
  }

  void DebounceInClass::setCallback(callback_t callback)
  {
    _callback = callback;
  }

  void DebounceInClass::interrupt_callback()
  {
    if (_task_handle != nullptr)
    {
      // 通知値を_max_polling_countに設定して通知
      xTaskNotifyFromISR(_task_handle, _max_polling_count, eSetValueWithOverwrite, nullptr);
    }
  }

  bool DebounceInClass::needsUpdate()
  {
    // ・消費電流を減らすため常にスキャンをせずに割り込みが発生したら起きて一定時間スキャン（ポーリング）をする
    // ・FreeRTOSのtask通知を利用して残りのポーリング回数を設定する
    if (ulTaskNotifyTake(pdFALSE, 0) > 0)
    {
      return true;
    }
    return false;
  }

  void DebounceInClass::task(void *pvParameters)
  {
    while (true)
    {
      if (needsUpdate())
      {
        // update
        for (int i = 0; i < _bounce_list_len; i++)
        {
          int pin = _bounce_list[i].getPin();
          if (_bounce_list[i].update())
          {
            if (_callback != nullptr)
            {
              _callback(pin, _bounce_list[i].read());
            }
          }
#if DEBOUNCE_IN_USE_SENSE_INTERRUPT == true
          if (readLatch(pin))
          {
            xTaskNotify(_task_handle, _max_polling_count, eSetValueWithOverwrite);
            clearLatch(pin);
          }
#endif
        }
        delay(_polling_interval_ms);
      }
      else
      {
        // 割り込みが発生するまで寝る
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
      }
    }
  }

  DebounceInClass DebounceIn;

} // namespace hidpg
