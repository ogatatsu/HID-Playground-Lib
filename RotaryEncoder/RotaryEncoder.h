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

  const uint32_t ROTARY_ENCODER_DOWNSHIFT_TIME_MS = 10000;
  const uint32_t ROTARY_ENCODER_REST_MODE_INTERVAL_MS = 100;
  const uint32_t ROTARY_ENCODER_REST_MODE_SCAN_INTERRUPT_WINDOW_MS = 1;

  template <uint8_t ID>
  class RotaryEncoder
  {
  public:
    using callback_t = void (*)(void);

    RotaryEncoder()
    {
      _timer_handle = xTimerCreateStatic("RotaryEncoder", 0, false, nullptr, timer_callback, &_timer_buffer);
    }

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

      Mode mode = _current_mode == Mode::Rest ? Mode::Run_Downshift : _current_mode;
      setMode(mode);

      if (_task_handle == nullptr)
      {
        _task_handle = xTaskCreateStatic(task, "RotaryEncoder", ROTARY_ENCODER_TASK_STACK_SIZE, nullptr, ROTARY_ENCODER_TASK_PRIO, _task_stack, &_task_tcb);
      }
      else
      {
        vTaskResume(_task_handle);
      }
    }

    void stop()
    {
      if (_qdec.getIsStarted() == false)
      {
        return;
      }

      vTaskSuspend(_task_handle);
      interrupt_off();
      xTimerStop(_timer_handle, portMAX_DELAY);
      _qdec.end();
    }

    void setCallback(callback_t cb)
    {
      _callback = cb;
    }

    void enableRestMode(bool is_rest_mode)
    {
      if (is_rest_mode == true && _current_mode == Mode::Run)
      {
        setMode(Mode::Run_Downshift);
      }
      else if (is_rest_mode == false && _current_mode != Mode::Run)
      {
        setMode(Mode::Run);
      }
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

  private:
    enum class Mode
    {
      Run,
      Run_Downshift,
      Rest,
    };

    static void interrupt_on()
    {
      pinMode(_qdec.getPinA(), INPUT_PULLUP);
      pinMode(_qdec.getPinB(), INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(_qdec.getPinA()), interrupt_callback, CHANGE);
      attachInterrupt(digitalPinToInterrupt(_qdec.getPinB()), interrupt_callback, CHANGE);
    }

    static void interrupt_off()
    {
      detachInterrupt(digitalPinToInterrupt(_qdec.getPinA()));
      detachInterrupt(digitalPinToInterrupt(_qdec.getPinB()));
      pinMode(_qdec.getPinA(), INPUT);
      pinMode(_qdec.getPinB(), INPUT);
    }

    static void setMode(Mode mode)
    {
      switch (mode)
      {
      case Mode::Run:
        xTimerStop(_timer_handle, portMAX_DELAY);
        _current_mode = Mode::Run;
        interrupt_on();
        break;

      case Mode::Run_Downshift:
        xTimerChangePeriod(_timer_handle, ROTARY_ENCODER_DOWNSHIFT_TIME_MS, portMAX_DELAY);
        _current_mode = Mode::Run_Downshift;
        interrupt_on();
        break;

      case Mode::Rest:
        xTimerChangePeriod(_timer_handle, ROTARY_ENCODER_REST_MODE_INTERVAL_MS, portMAX_DELAY);
        interrupt_off();
        _current_mode = Mode::Rest;
        break;

      default:
        break;
      }
    }

    static void timer_callback(TimerHandle_t timer_handle)
    {
      if (_current_mode == Mode::Run_Downshift)
      {
        setMode(Mode::Rest);
      }
      else if (_current_mode == Mode::Rest)
      {
        _does_interrupt = false;
        interrupt_on();
        delay(ROTARY_ENCODER_REST_MODE_SCAN_INTERRUPT_WINDOW_MS);
        interrupt_off();
        if (_does_interrupt)
        {
          setMode(Mode::Run_Downshift);
        }
        else
        {
          xTimerChangePeriod(_timer_handle, ROTARY_ENCODER_REST_MODE_INTERVAL_MS, 0);
        }
      }
    }

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

        if (_current_mode == Mode::Run_Downshift)
        {
          xTimerResetFromISR(_timer_handle, nullptr);
        }
        else if (_current_mode == Mode::Rest)
        {
          _does_interrupt = true;
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
    static Mode _current_mode;
    static bool _does_interrupt;
    static TimerHandle_t _timer_handle;
    static StaticTimer_t _timer_buffer;
  };

  template <uint8_t ID>
  typename RotaryEncoder<ID>::callback_t RotaryEncoder<ID>::_callback = nullptr;

  template <uint8_t ID>
  typename RotaryEncoder<ID>::Mode RotaryEncoder<ID>::_current_mode = RotaryEncoder<ID>::Mode::Run;

  template <uint8_t ID>
  bool RotaryEncoder<ID>::_does_interrupt = false;

  template <uint8_t ID>
  TimerHandle_t RotaryEncoder<ID>::_timer_handle = nullptr;

  template <uint8_t ID>
  StaticTimer_t RotaryEncoder<ID>::_timer_buffer;

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
