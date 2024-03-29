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

#pragma once

#include "Arduino.h"
#include "BlinkLed_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

namespace hidpg
{

  class BlinkLed
  {
  public:
    BlinkLed(uint8_t pin, uint8_t active_state = HIGH);

    void begin();
    void blink(uint8_t n_times = 1);
    void off();
    void waitSyncOff();
    bool isBlink() const;
    void suspend();
    void resume();
    bool isSuspended() const;

  private:
    static void task(void *pvParameters);

    const uint8_t _pin;
    const uint8_t _active_state;
    volatile bool _is_blink;
    volatile uint8_t _n_times;
    volatile bool _is_suspend;
    TaskHandle_t _task_handle;
    StackType_t _task_stack[BLINK_LED_TASK_STACK_SIZE];
    StaticTask_t _task_tcb;
    SemaphoreHandle_t _mutex;
    StaticSemaphore_t _mutex_buffer;
  };

} // namespace hidpg
