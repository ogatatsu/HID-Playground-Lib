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

#include "MatrixScan.h"

namespace hidpg
{

MatrixScan::callback_t MatrixScan::_callback = nullptr;
TaskHandle_t MatrixScan::_taskHandle = nullptr;
uint16_t MatrixScan::_maxDebounceDelay = 0;
uint16_t MatrixScan::_minDebounceDelay = UINT16_MAX;
Switch **MatrixScan::_matrix = nullptr;
const uint8_t *MatrixScan::_inPins = nullptr;
const uint8_t *MatrixScan::_outPins = nullptr;
uint8_t MatrixScan::_inLength = 0;
uint8_t MatrixScan::_outLength = 0;

void MatrixScan::setKeyscanCallback(callback_t callback)
{
  _callback = callback;
}

void MatrixScan::startTask()
{
  if (_taskHandle == nullptr)
  {
    xTaskCreate(task, "MatrixScan", MATRIX_SCAN_TASK_STACK_SIZE, nullptr, MATRIX_SCAN_TASK_PRIO, &_taskHandle);
  }
}

#ifdef ARDUINO_ARCH_NRF52
void MatrixScan::stopTask_and_setWakeUpInterrupt()
{
  if (_taskHandle != nullptr)
  {
    vTaskSuspend(_taskHandle);
  }

  outPinsSet(ACTIVE_STATE);

  for (int i = 0; i < _inLength; i++)
  {
    NRF_GPIO->PIN_CNF[_inPins[i]] |= ((uint32_t)(ACTIVE_STATE ? GPIO_PIN_CNF_SENSE_High : GPIO_PIN_CNF_SENSE_Low) << GPIO_PIN_CNF_SENSE_Pos);
  }
}
#endif

// ピンの初期化など
void MatrixScan::initMatrix()
{
  // 出力ピンの設定
  for (int i = 0; i < _outLength; i++)
  {
    pinMode(_outPins[i], OUTPUT);
    digitalWrite(_outPins[i], ACTIVE_STATE);
  }
  // 入力ピンの設定
  for (int i = 0; i < _inLength; i++)
  {
    pinMode(_inPins[i], (ACTIVE_STATE ? INPUT_PULLDOWN : INPUT_PULLUP));
    attachInterrupt(_inPins[i], interrupt_callback, (ACTIVE_STATE ? RISING : FALLING));
  }

  // スイッチオブジェクトの初期化
  for (int o = 0; o < _outLength; o++)
  {
    for (int i = 0; i < _inLength; i++)
    {
      int idx = o * _inLength + i;
      if (_matrix[idx] == nullptr)
      {
        continue;
      }
      _matrix[idx]->init(_inPins[i]);
      uint16_t d = _matrix[idx]->debounceDelay();
      _maxDebounceDelay = max(d, _maxDebounceDelay);
      _minDebounceDelay = min(d, _minDebounceDelay);
    }
  }
}

// 起きる
void MatrixScan::interrupt_callback()
{
  if (_taskHandle != nullptr)
  {
    xTaskNotifyFromISR(_taskHandle, 2, eSetValueWithOverwrite, nullptr);
  }
}

// 出力ピンを一括設定
void MatrixScan::outPinsSet(int val)
{
  for (int i = 0; i < _outLength; i++)
  {
    digitalWrite(_outPins[i], val);
  }
}

// 割り込みが発生してから+(maxDebounceDelay * 3)msまでの間はスキャンする。
// キー押し時はスキャン中に入力ピンの電圧が変わるので押されてる限り割り込みが発生し続ける
bool MatrixScan::needsKeyscan()
{
  static unsigned long lastInterruptMillis = 0;

  // 割り込みされたら
  if (ulTaskNotifyTake(pdTRUE, 0) > 0)
  {
    lastInterruptMillis = millis();
    return true;
  }
  // 最後に押されてた時間から今の時間までを計算して
  unsigned long currentMillis = millis();
  if ((unsigned long)(currentMillis - lastInterruptMillis) <= (_maxDebounceDelay * 3))
  {
    return true;
  }
  return false;
}

void MatrixScan::task(void *pvParameters)
{
  Set currentIDs, previousIDs;

  while (1)
  {
    if (needsKeyscan())
    {
      // スキャン
      outPinsSet(!ACTIVE_STATE);
      for (int o = 0; o < _outLength; o++)
      {
        digitalWrite(_outPins[o], ACTIVE_STATE);
        for (int i = 0; i < _inLength; i++)
        {
          int idx = o * _inLength + i;
          if (_matrix[idx] == nullptr)
          {
            continue;
          }
          _matrix[idx]->updateState(currentIDs);
        }
        digitalWrite(_outPins[o], !ACTIVE_STATE);
      }
      // 割り込みのためにスキャンが終わったら出力をアクティブ側に設定
      outPinsSet(ACTIVE_STATE);

      // 更新してたらコールバック関数を発火
      if (currentIDs != previousIDs)
      {
        if (_callback != nullptr)
        {
          _callback(currentIDs);
        }
        previousIDs = currentIDs;
      }
      // poll interval
      delay(_minDebounceDelay);
    }
    else
    {
      // 割り込みが発生するまで寝る
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    }
  }
}

} // namespace hidpg
