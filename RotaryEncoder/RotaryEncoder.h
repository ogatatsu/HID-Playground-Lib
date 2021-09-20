/*
  The MIT License (MIT)

  Copyright (c) 2021 ogatatsu.

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

#include "Arduino.h"
#include "RotaryEncoder_config.h"
#include "qdec.h"

namespace hidpg
{

  template <uint8_t ID>
  class RotaryEncoder
  {
  public:
    using callback_t = void (*)(void);

    void setPins(uint16_t pin_a, uint16_t pin_b)
    {
      _qdec.setPinA(pin_a);
      _qdec.setPinB(pin_b);
    }

    void useFullStep(bool is_full_step)
    {
      _qdec.setFullStep(is_full_step);
    }

    void start()
    {
      if (_qdec.getIsStarted())
      {
        return;
      }

      _qdec.begin();

      attachInterrupt(digitalPinToInterrupt(_qdec.getPinA()), interrupt_callback, CHANGE);
      attachInterrupt(digitalPinToInterrupt(_qdec.getPinB()), interrupt_callback, CHANGE);

      if (_task_handle == nullptr)
      {
        _task_handle = xTaskCreateStatic(task, "RotaryEncoder", ROTARY_ENCODER_TASK_STACK_SIZE, nullptr, ROTARY_ENCODER_TASK_PRIO, _task_stack, &_task_tcb);
      }
      else
      {
        vTaskResume(_task_handle);
      }
    }

    void setCallback(callback_t cb)
    {
      _callback = cb;
    }

    int32_t readStep()
    {
      taskENTER_CRITICAL();
      int32_t result = _step;
      _step = 0;
      _needs_call = true;
      taskEXIT_CRITICAL();

      return result;
    }

    void stop()
    {
      if (_qdec.getIsStarted() == false)
      {
        return;
      }

      vTaskSuspend(_task_handle);

      detachInterrupt(digitalPinToInterrupt(_qdec.getPinA()));
      detachInterrupt(digitalPinToInterrupt(_qdec.getPinB()));

      pinMode(_qdec.getPinA(), INPUT);
      pinMode(_qdec.getPinB(), INPUT);

      _qdec.end();
    }

  private:
    static void task(void *pvParameters)
    {
      while (true)
      {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (_callback != nullptr)
        {
          _callback();
        }
      }
    };

    static void interrupt_callback()
    {
      if (_task_handle != nullptr)
      {
        using namespace ::SimpleHacks;
        QDECODER_EVENT event = _qdec.update();
        if (event & QDECODER_EVENT_CW)
        {
          _step++;
          if (_needs_call == true)
          {
            xTaskNotifyFromISR(_task_handle, 1, eSetValueWithOverwrite, nullptr);
            _needs_call = false;
          }
        }
        else if (event & QDECODER_EVENT_CCW)
        {
          _step--;
          if (_needs_call == true)
          {
            xTaskNotifyFromISR(_task_handle, 1, eSetValueWithOverwrite, nullptr);
            _needs_call = false;
          }
        }
      }
    }

    static callback_t _callback;
    static bool _needs_call;
    static SimpleHacks::QDecoder _qdec;
    static int32_t _step;
    static TaskHandle_t _task_handle;
    static StackType_t _task_stack[];
    static StaticTask_t _task_tcb;
  };

  template <uint8_t ID>
  typename RotaryEncoder<ID>::callback_t RotaryEncoder<ID>::_callback = nullptr;

  template <uint8_t ID>
  bool RotaryEncoder<ID>::_needs_call = true;

  template <uint8_t ID>
  SimpleHacks::QDecoder RotaryEncoder<ID>::_qdec;

  template <uint8_t ID>
  int32_t RotaryEncoder<ID>::_step = 0;

  template <uint8_t ID>
  TaskHandle_t RotaryEncoder<ID>::_task_handle = nullptr;

  template <uint8_t ID>
  StackType_t RotaryEncoder<ID>::_task_stack[ROTARY_ENCODER_TASK_STACK_SIZE];

  template <uint8_t ID>
  StaticTask_t RotaryEncoder<ID>::_task_tcb;

} // namespace hidpg
