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
#include "SenseInterrupt.h"

namespace hidpg
{

  MatrixScanClass::callback_t MatrixScanClass::_callback = nullptr;
  TaskHandle_t MatrixScanClass::_task_handle = nullptr;
  uint16_t MatrixScanClass::_polling_interval_ms = 0;
  uint16_t MatrixScanClass::_max_polling_count = 0;
  Switch **MatrixScanClass::_matrix = nullptr;
  const uint8_t *MatrixScanClass::_in_pins = nullptr;
  const uint8_t *MatrixScanClass::_out_pins = nullptr;
  uint8_t MatrixScanClass::_in_pins_len = 0;
  uint8_t MatrixScanClass::_out_pins_len = 0;

  void MatrixScanClass::setCallback(callback_t callback)
  {
    _callback = callback;
  }

#ifdef ARDUINO_ARCH_NRF52
  void MatrixScanClass::stopTask_and_setWakeUpInterrupt()
  {
    if (_task_handle != nullptr)
    {
      vTaskSuspend(_task_handle);
    }

    outPinsSet(ACTIVE_STATE);

    for (int i = 0; i < _in_pins_len; i++)
    {
      NRF_GPIO->PIN_CNF[_in_pins[i]] |= ((uint32_t)(ACTIVE_STATE ? GPIO_PIN_CNF_SENSE_High : GPIO_PIN_CNF_SENSE_Low) << GPIO_PIN_CNF_SENSE_Pos);
    }
  }
#endif

  // ピンの初期化など
  void MatrixScanClass::begin()
  {
    // 出力ピンの設定
    for (int i = 0; i < _out_pins_len; i++)
    {
      pinMode(_out_pins[i], OUTPUT);
      digitalWrite(_out_pins[i], ACTIVE_STATE);
    }

    // 入力ピンの設定
    for (int i = 0; i < _in_pins_len; i++)
    {
#if (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == false)
      pinMode(_in_pins[i], INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(_in_pins[i]), interrupt_callback, FALLING);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == false)
      pinMode(_in_pins[i], INPUT_PULLDOWN);
      attachInterrupt(digitalPinToInterrupt(_in_pins[i]), interrupt_callback, RISING);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == false)
      pinMode(_in_pins[i], INPUT);
      attachInterrupt(digitalPinToInterrupt(_in_pins[i]), interrupt_callback, FALLING);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == false)
      pinMode(_in_pins[i], INPUT);
      attachInterrupt(digitalPinToInterrupt(_in_pins[i]), interrupt_callback, RISING);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == true)
      pinMode(_in_pins[i], INPUT_PULLUP_SENSE);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == false) && (USE_SENSE_INTERRUPT == true)
      pinMode(_in_pins[i], INPUT_PULLDOWN_SENSE);
#elif (ACTIVE_STATE == LOW) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == true)
      pinMode(_in_pins[i], INPUT_SENSE_LOW);
#elif (ACTIVE_STATE == HIGH) && (USE_EXTERNAL_PULL_RESISTOR == true) && (USE_SENSE_INTERRUPT == true)
      pinMode(_in_pins[i], INPUT_SENSE_HIGH);
#endif
    }

#if (USE_SENSE_INTERRUPT == true)
    attachSenseInterrupt(interrupt_callback);
#endif

    uint16_t max_debounce_delay_ms = 0;
    uint16_t min_debounce_delay_ms = UINT16_MAX;

    // スイッチオブジェクトの初期化
    for (int oi = 0; oi < _out_pins_len; oi++)
    {
      for (int ii = 0; ii < _in_pins_len; ii++)
      {
        int idx = oi * _in_pins_len + ii;
        if (_matrix[idx] == nullptr)
        {
          continue;
        }
        _matrix[idx]->attach(_in_pins[ii]);
        uint16_t d = _matrix[idx]->getDebounceDelay();
        max_debounce_delay_ms = max(d, max_debounce_delay_ms);
        min_debounce_delay_ms = min(d, min_debounce_delay_ms);
      }
    }
    // debounce_delayの最小値の間隔でポーリングする
    _polling_interval_ms = min_debounce_delay_ms;
    // ポーリング回数は最低でもdebounce_delayの最大値を超える値に設定
    _max_polling_count = (max_debounce_delay_ms + (_polling_interval_ms - 1)) / _polling_interval_ms; // ceil(max_debounce_delay_ms / _polling_interval_ms) 相当
    _max_polling_count += 2;

    xTaskCreate(task, "MatrixScan", MATRIX_SCAN_TASK_STACK_SIZE, nullptr, MATRIX_SCAN_TASK_PRIO, &_task_handle);
  }

  // 起きる
  void MatrixScanClass::interrupt_callback()
  {
    if (_task_handle != nullptr)
    {
      // 通知値を_max_polling_countに設定して通知
      xTaskNotifyFromISR(_task_handle, _max_polling_count, eSetValueWithOverwrite, nullptr);
    }
  }

  // 出力ピンを一括設定
  void MatrixScanClass::outPinsSet(int val)
  {
    for (int i = 0; i < _out_pins_len; i++)
    {
      digitalWrite(_out_pins[i], val);
    }
  }

  bool MatrixScanClass::needsKeyScan()
  {
    // - 消費電流を減らすため常にスキャンをせずに割り込みが発生したら起きて一定時間スキャン（ポーリング）をする
    // - FreeRTOSのtask通知を利用して残りのポーリング回数を設定する
    // - キー押し時はスキャン中に入力ピンの電圧が変わるので押されてる限り割り込みが発生し続ける
    // - 割り込みが発生し続けてる間（キーが押され続けている間）はtaskの通知値は_max_polling_countに設定され続ける
    // - キーが離されたら割り込みが発生しなくなりそこから_max_polling_count回ポーリングされ0になったらスキャンを終了
    if (ulTaskNotifyTake(pdFALSE, 0) > 0)
    {
      return true;
    }
    return false;
  }

  void MatrixScanClass::task(void *pvParameters)
  {
    Set ids, prev_ids;

    while (true)
    {
      if (needsKeyScan())
      {
        // スキャン
        outPinsSet(!ACTIVE_STATE);
        for (int oi = 0; oi < _out_pins_len; oi++)
        {
          digitalWrite(_out_pins[oi], ACTIVE_STATE);
          for (int ii = 0; ii < _in_pins_len; ii++)
          {
            int idx = oi * _in_pins_len + ii;
            if (_matrix[idx] == nullptr)
            {
              continue;
            }
            _matrix[idx]->updateState(ids);
          }
          digitalWrite(_out_pins[oi], !ACTIVE_STATE);
        }
        // 割り込みのためにスキャンが終わったら出力をアクティブ側に設定
        outPinsSet(ACTIVE_STATE);

        // 更新してたらコールバック関数を発火
        if (ids != prev_ids)
        {
          if (_callback != nullptr)
          {
            _callback(ids);
          }
          prev_ids = ids;
        }
        delay(_polling_interval_ms);
      }
      else
      {
        // 割り込みが発生するまで寝る
        ulTaskNotifyTake(pdFALSE, portMAX_DELAY);
      }
    }
  }

  MatrixScanClass MatrixScan;

} // namespace hidpg
