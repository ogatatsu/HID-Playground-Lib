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

#include "Set.h"
#include "Switch.h"
#include "portFreeRTOS.h"

namespace hidpg
{

class MatrixScan
{
public:
  using callback_t = void (*)(const Set &switchIDs);

  template <uint8_t outLength, uint8_t inLength>
  static void setMatrix(Switch *matrix[outLength][inLength], const uint8_t (&outPins)[outLength], const uint8_t (&inPins)[inLength])
  {
    _inLength = inLength;
    _outLength = outLength;
    _inPins = inPins;
    _outPins = outPins;
    _matrix = reinterpret_cast<Switch **>(matrix);
  }
  static void init();
  static void setKeyscanCallback(callback_t callback);
  static void startTask();

#ifdef ARDUINO_ARCH_NRF52
  static void stopTask_and_setWakeUpInterrupt();
#endif

private:
  static void interrupt_callback();
  static void outPinsSet(int val);
  static bool needsKeyscan();
  static void task(void *pvParameters);

  static callback_t _callback;
  static TaskHandle_t _taskHandle;
  static uint16_t _maxDebounceDelay;
  static uint16_t _minDebounceDelay;
  static Switch **_matrix;
  static const uint8_t *_inPins;
  static const uint8_t *_outPins;
  static uint8_t _inLength;
  static uint8_t _outLength;
};

} // namespace hidpg
