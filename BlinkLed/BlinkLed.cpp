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
#include "BlinkLed_config.h"

namespace hidpg
{

#ifdef ARDUINO_ARCH_NRF52
  static void pinMode_OutputEx(uint32_t pin, bool is_high_drive)
  {
    if (pin >= PINS_COUNT)
    {
      return;
    }

    pin = g_ADigitalPinMap[pin];

    uint32_t drive_mode = is_high_drive ? GPIO_PIN_CNF_DRIVE_H0H1 : GPIO_PIN_CNF_DRIVE_S0S1;

    NRF_GPIO->PIN_CNF[pin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                             (drive_mode << GPIO_PIN_CNF_DRIVE_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
  }
#endif

  BlinkLed::BlinkLed(uint8_t pin, uint8_t active_state, bool is_high_drive)
      : _pin(pin), _active_state(active_state), _is_blink(false), _times(0)
  {

#ifdef ARDUINO_ARCH_NRF52
    pinMode_OutputEx(pin, is_high_drive);
#else
    pinMode(pin, OUTPUT);
#endif

    char task_name[] = "Led000";
    task_name[3] += pin / 100;
    task_name[4] += pin % 100 / 10;
    task_name[5] += pin % 10;

    xTaskCreate(task, task_name, BLINK_LED_TASK_STACK_SIZE, this, BLINK_LED_TASK_PRIO, &_task_handle);
  }

  void BlinkLed::task(void *pvParameters)
  {
    BlinkLed *that = static_cast<BlinkLed *>(pvParameters);

    while (true)
    {
      that->_is_blink = false;
      xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
      while (that->_times != 0)
      {
        that->_is_blink = true;
        for (int i = 0; i < that->_times; i++)
        {
          digitalWrite(that->_pin, that->_active_state);
          delay(BLINK_LED_INTERVAL_MS);
          digitalWrite(that->_pin, !that->_active_state);
          delay(BLINK_LED_INTERVAL_MS);
        }
        delay(BLINK_LED_INTERVAL_MS * 3);
      }
    }
  }

  void BlinkLed::blink(uint8_t times)
  {
    _times = times;
    xTaskNotify(_task_handle, 0, eNoAction);
  }

  void BlinkLed::off()
  {
    _times = 0;
  }

  void BlinkLed::syncOff()
  {
    _times = 0;
    while (_is_blink)
    {
      delay(1);
    }
  }

  bool BlinkLed::isBlink() const
  {
    return _is_blink;
  }

} // namespace hidpg
