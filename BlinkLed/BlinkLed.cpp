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

#include "BlinkLed.h"

namespace hidpg
{

  BlinkLed::BlinkLed(uint8_t pin, uint8_t active_state)
      : _pin(pin), _active_state(active_state), _is_blink(false), _n_times(0), _is_suspend(false)
  {
  }

  void BlinkLed::begin()
  {
#ifdef ARDUINO_ARCH_NRF52
    if (_active_state == HIGH)
      pinMode(_pin, OUTPUT_D0H1);
    else
      pinMode(_pin, OUTPUT_H0D1);
#else
    pinMode(_pin, OUTPUT);
#endif

    char task_name[] = "Led000";
    task_name[3] += _pin / 100;
    task_name[4] += _pin % 100 / 10;
    task_name[5] += _pin % 10;

    _mutex = xSemaphoreCreateMutexStatic(&_mutex_buffer);
    _task_handle = xTaskCreateStatic(task, task_name, BLINK_LED_TASK_STACK_SIZE, this, BLINK_LED_TASK_PRIO, _task_stack, &_task_tcb);
  }

  void BlinkLed::task(void *pvParameters)
  {
    BlinkLed *that = static_cast<BlinkLed *>(pvParameters);

    while (true)
    {
    EARLY_STOP:
      that->_is_blink = false;
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

      while (that->_n_times != 0 && that->_is_suspend == false)
      {
        that->_is_blink = true;
        for (int i = 0; i < that->_n_times; i++)
        {
          digitalWrite(that->_pin, that->_active_state);
          vTaskDelay(pdMS_TO_TICKS(BLINK_LED_INTERVAL_MS));
          digitalWrite(that->_pin, !that->_active_state);
          if (that->_n_times == 0 || that->_is_suspend)
          {
            goto EARLY_STOP;
          }
          vTaskDelay(pdMS_TO_TICKS(BLINK_LED_INTERVAL_MS));
          if (that->_n_times == 0 || that->_is_suspend)
          {
            goto EARLY_STOP;
          }
        }
        vTaskDelay(pdMS_TO_TICKS(BLINK_LED_INTERVAL_MS));
        if (that->_n_times == 0 || that->_is_suspend)
        {
          goto EARLY_STOP;
        }
        vTaskDelay(pdMS_TO_TICKS(BLINK_LED_INTERVAL_MS));
        if (that->_n_times == 0 || that->_is_suspend)
        {
          goto EARLY_STOP;
        }
        vTaskDelay(pdMS_TO_TICKS(BLINK_LED_INTERVAL_MS));
      }
    }
  }

  void BlinkLed::blink(uint8_t n_times)
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _n_times = n_times;
    xTaskNotify(_task_handle, 1, eSetValueWithOverwrite);
    xSemaphoreGive(_mutex);
  }

  void BlinkLed::off()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _n_times = 0;
    xSemaphoreGive(_mutex);
  }

  void BlinkLed::waitSyncOff()
  {
    while (_is_blink)
    {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  bool BlinkLed::isBlink() const
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    bool result = _is_blink;
    xSemaphoreGive(_mutex);

    return result;
  }

  void BlinkLed::suspend()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _is_suspend = true;
    xSemaphoreGive(_mutex);
  }

  void BlinkLed::resume()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    if (_is_suspend)
    {
      _is_suspend = false;
      if (_n_times != 0)
      {
        xTaskNotify(_task_handle, 1, eSetValueWithOverwrite);
      }
    }
    xSemaphoreGive(_mutex);
  }

  bool BlinkLed::isSuspended() const
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    bool result = _is_suspend;
    xSemaphoreGive(_mutex);

    return result;
  }

} // namespace hidpg
