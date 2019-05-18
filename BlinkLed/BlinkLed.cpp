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
#include <Arduino.h>

namespace hidpg
{

#ifdef ARDUINO_ARCH_NRF52
static void pinMode_OutputEx(uint32_t ulPin, bool isHighDrive)
{
  if (ulPin >= PINS_COUNT)
  {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  uint32_t driveMode = isHighDrive ? GPIO_PIN_CNF_DRIVE_H0H1 : GPIO_PIN_CNF_DRIVE_S0S1;

  NRF_GPIO->PIN_CNF[ulPin] = ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                             (driveMode << GPIO_PIN_CNF_DRIVE_Pos) |
                             ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}
#endif

BlinkLed::BlinkLed(uint8_t pin, bool isHighDrive) : _pin(pin), _isBlink(false), _times(0)
{

#ifdef ARDUINO_ARCH_NRF52
  pinMode_OutputEx(pin, isHighDrive);
#else
  pinMode(pin, OUTPUT);
#endif

  char taskName[] = "Led000";
  taskName[3] += pin / 100;
  taskName[4] += pin % 100 / 10;
  taskName[5] += pin % 10;

  xTaskCreate(task, taskName, BLINK_LED_TASK_STACK_SIZE, this, BLINK_LED_TASK_PRIO, &_taskHandle);
}

void BlinkLed::task(void *pvParameters)
{
  BlinkLed *that = static_cast<BlinkLed *>(pvParameters);

  while (true)
  {
    that->_isBlink = false;
    xTaskNotifyWait(0, 0, nullptr, portMAX_DELAY);
    while (that->_times != 0)
    {
      that->_isBlink = true;
      for (int i = 0; i < that->_times; i++)
      {
        digitalWrite(that->_pin, HIGH);
        delay(BLINK_LED_INTERVAL);
        digitalWrite(that->_pin, LOW);
        delay(BLINK_LED_INTERVAL);
      }
      delay(BLINK_LED_INTERVAL * 3);
    }
  }
}

void BlinkLed::blink(uint8_t times)
{
  _times = times;
  xTaskNotify(_taskHandle, 0, eNoAction);
}

void BlinkLed::off()
{
  _times = 0;
}

void BlinkLed::syncOff()
{
  _times = 0;
  while (_isBlink)
  {
    delay(1);
  }
}

bool BlinkLed::isBlink() const
{
  return _isBlink;
}

} // namespace hidpg
