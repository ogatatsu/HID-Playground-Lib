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
uint16_t MatrixScan::_pollingInterval = 0;
uint16_t MatrixScan::_pollingMax = 0;
Switch **MatrixScan::_matrix = nullptr;
const uint8_t *MatrixScan::_inPins = nullptr;
const uint8_t *MatrixScan::_outPins = nullptr;
uint8_t MatrixScan::_inLength = 0;
uint8_t MatrixScan::_outLength = 0;

void MatrixScan::setCallback(callback_t callback)
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
void MatrixScan::init()
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
#if (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == false)
    pinMode(_inPins[i], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_inPins[i]), interrupt_callback, FALLING);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == false)
    pinMode(_inPins[i], INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(_inPins[i]), interrupt_callback, RISING);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == false)
    pinMode(_inPins[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(_inPins[i]), interrupt_callback, FALLING);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == false)
    pinMode(_inPins[i], INPUT);
    attachInterrupt(digitalPinToInterrupt(_inPins[i]), interrupt_callback, RISING);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == true)
    pinModeEx(_inPins[i], INPUT_PULLUP, DRIVE_S0S1, SENSE_LOW);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == true)
    pinModeEx(_inPins[i], INPUT_PULLDOWN, DRIVE_S0S1, SENSE_HIGH);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == true)
    pinModeEx(_inPins[i], INPUT, DRIVE_S0S1, SENSE_LOW);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == true)
    pinModeEx(_inPins[i], INPUT, DRIVE_S0S1, SENSE_HIGH);
#endif
  }

#if (USE_SENSE_INTERRUPT == true)
  attachSenseInterrupt(interrupt_callback);
#endif

  uint16_t maxDebounceDelay = 0;
  uint16_t minDebounceDelay = UINT16_MAX;

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
      maxDebounceDelay = max(d, maxDebounceDelay);
      minDebounceDelay = min(d, minDebounceDelay);
    }
  }
  // debounceDelayの最小値の間隔でポーリングする
  _pollingInterval = minDebounceDelay;
  // ポーリング回数は最低でもdebounceDelayの最大値を超える値に設定
  _pollingMax = (maxDebounceDelay + (_pollingInterval - 1)) / _pollingInterval; // ceil(maxDebounceDelay / _pollingInterval)
  _pollingMax += 2;
}

// 起きる
void MatrixScan::interrupt_callback()
{
  if (_taskHandle != nullptr)
  {
    // 通知値を_pollingMaxに設定して通知
    xTaskNotifyFromISR(_taskHandle, _pollingMax, eSetValueWithOverwrite, nullptr);
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

bool MatrixScan::needsKeyscan()
{
  // ・消費電流を減らすため常にスキャンをせずに割り込みが発生したら起きて一定時間スキャン（ポーリング）をする
  // ・taskの通知値を利用して残りのポーリング回数を設定する
  // ・キー押し時はスキャン中に入力ピンの電圧が変わるので押されてる限り割り込みが発生し続ける
  // ・割り込みが発生し続けてる間（キーが押され続けている間）はtaskの通知値は_pollingMaxに設定され続ける
  // ・キーが離されたら割り込みが発生しなくなりそこから_pollingMax回ポーリングされ0になったらスキャンを終了
  if (ulTaskNotifyTake(pdFALSE, 0) > 0)
  {
    return true;
  }
  return false;
}

void MatrixScan::task(void *pvParameters)
{
  Set currentIDs, previousIDs;

  while (true)
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
      delay(_pollingInterval);
    }
    else
    {
      // 割り込みが発生するまで寝る
      ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
    }
  }
}

} // namespace hidpg
