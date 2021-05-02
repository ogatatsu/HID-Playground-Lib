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

#include "FreeRTOS.h"
#include "Set.h"
#include "Switch.h"
#include "task.h"

namespace hidpg
{

  class MatrixScanClass
  {
  public:
    using callback_t = void (*)(const Set &switch_ids);

    template <uint8_t out_pins_len, uint8_t in_pins_len>
    static void setMatrix(Switch *matrix[out_pins_len][in_pins_len], const uint8_t (&out_pins)[out_pins_len], const uint8_t (&in_pins)[in_pins_len])
    {
      _in_pins_len = in_pins_len;
      _out_pins_len = out_pins_len;
      _in_pins = in_pins;
      _out_pins = out_pins;
      _matrix = reinterpret_cast<Switch **>(matrix);
    }
    static void begin();
    static void setCallback(callback_t callback);

#ifdef ARDUINO_ARCH_NRF52
    static void stopTask_and_setWakeUpInterrupt();
#endif

  private:
    static void interrupt_callback();
    static void outPinsSet(int val);
    static bool needsKeyScan();
    static void task(void *pvParameters);

    static callback_t _callback;
    static uint16_t _polling_interval_ms;
    static uint16_t _max_polling_count;
    static Switch **_matrix;
    static const uint8_t *_in_pins;
    static const uint8_t *_out_pins;
    static uint8_t _in_pins_len;
    static uint8_t _out_pins_len;

    static TaskHandle_t _task_handle;
    static StackType_t _task_stack[];
    static StaticTask_t _task_tcb;
  };

  extern MatrixScanClass MatrixScan;

} // namespace hidpg
