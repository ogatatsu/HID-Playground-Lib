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
#include "FreeRTOS.h"
#include "RotaryEncoder_config.h"
#include "qdec.h"
#include "task.h"

namespace hidpg
{

  namespace Internal
  {
    template <uint8_t ID>
    class RotaryEncoder_InterruptCallback
    {
    public:
      static void interrupt_callback()
      {
        if (_task_handle != nullptr)
        {
          using namespace ::SimpleHacks;
          QDECODER_EVENT event = _qdec().update();
          if (event & QDECODER_EVENT_CW)
          {
#if configNUM_CORES > 1
            taskENTER_CRITICAL_FROM_ISR();
#endif
            _step++;
            if (_needs_call == true)
            {
              xTaskNotifyFromISR(_task_handle, 1, eSetValueWithOverwrite, nullptr);
              _needs_call = false;
            }
#if configNUM_CORES > 1
            taskEXIT_CRITICAL_FROM_ISR();
#endif
          }
          else if (event & QDECODER_EVENT_CCW)
          {
#if configNUM_CORES > 1
            taskENTER_CRITICAL_FROM_ISR();
#endif
            _step--;
            if (_needs_call == true)
            {
              xTaskNotifyFromISR(_task_handle, 1, eSetValueWithOverwrite, nullptr);
              _needs_call = false;
            }
#if configNUM_CORES > 1
            taskEXIT_CRITICAL_FROM_ISR();
#endif
          }
        }
      }

      static SimpleHacks::QDecoder &_qdec()
      {
        static SimpleHacks::QDecoder qdec;
        return qdec;
      };

      static TaskHandle_t _task_handle;
      static int32_t _step;
      static bool _needs_call;
    };

    template <uint8_t ID>
    TaskHandle_t RotaryEncoder_InterruptCallback<ID>::_task_handle = nullptr;

    template <uint8_t ID>
    int32_t RotaryEncoder_InterruptCallback<ID>::_step = 0;

    template <uint8_t ID>
    bool RotaryEncoder_InterruptCallback<ID>::_needs_call = true;

  } // namespace Internal

  class RotaryEncoder
  {
  public:
    using callback_t = void (*)(void);

    template <uint8_t ID>
    static RotaryEncoder &create(uint16_t pin_a, uint16_t pin_b)
    {
      static RotaryEncoder instance(pin_a,
                                    pin_b,
                                    Internal::RotaryEncoder_InterruptCallback<ID>::_task_handle,
                                    Internal::RotaryEncoder_InterruptCallback<ID>::interrupt_callback,
                                    Internal::RotaryEncoder_InterruptCallback<ID>::_qdec(),
                                    Internal::RotaryEncoder_InterruptCallback<ID>::_step,
                                    Internal::RotaryEncoder_InterruptCallback<ID>::_needs_call);

      return instance;
    }

    void useFullStep(bool is_full_step);
    void start();
    void setCallback(callback_t cb);
    int32_t readStep();
    void stop();

  private:
    RotaryEncoder(uint16_t pin_a,
                  uint16_t pin_b,
                  TaskHandle_t &task_handle,
                  voidFuncPtr interrupt_callback,
                  SimpleHacks::QDecoder &qdec,
                  int32_t &step,
                  bool &_needs_call);

    static void task(void *pvParameters);

    TaskHandle_t &_task_handle;
    StackType_t _task_stack[ROTARY_ENCODER_TASK_STACK_SIZE];
    StaticTask_t _task_tcb;
    voidFuncPtr _interrupt_callback;
    SimpleHacks::QDecoder &_qdec;
    int32_t &_step;
    bool &_needs_call;
    callback_t _callback;
  };

} // namespace hidpg
