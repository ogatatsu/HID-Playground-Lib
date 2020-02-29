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

#include "Bounce2.h"
#include "LinkedList.h"
#include "freeRTOS.h"
#include <stdint.h>

namespace hidpg
{

class DebounceIn_
{
public:
  using callback_t = void (*)(uint8_t pin, bool state);

  static void init();
  static void addPin(uint8_t pin, int mode, uint16_t debounce_delay_ms = 10);
  static void setCallback(callback_t callback);
  static void startTask();
  static void stopTask();

private:
  struct PinInfo
  {
    uint8_t pin;
    int mode;
    Bounce bounce;
  };

  static void task(void *pvParameters);
  static bool needsUpdate();
  static void interrupt_callback();

  static TaskHandle_t _task_handle;
  static callback_t _callback;
  static LinkedList<PinInfo *> _pin_info_list;
  static uint16_t _max_debounce_delay_ms;
  static uint16_t _polling_interval_ms;
  static uint16_t _max_polling_count;
};

extern DebounceIn_ DebounceIn;

} // namespace hidpg
