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

// cherry mx bounce time is <= 5ms
#ifndef DEBOUNCE_DELAY_MS
#define DEBOUNCE_DELAY_MS 6
#endif

// スイッチがHIGHとLOWどちらでONになるか
#ifndef ACTIVE_STATE
#define ACTIVE_STATE LOW
#endif

// 外部のプル抵抗を使うか
#ifndef USE_EXTERNAL_PULL_RESISTOR
#define USE_EXTERNAL_PULL_RESISTOR false
#endif

// 割り込みにSense signalを使用するか
// nRF52でなおかつcustom version frameworkでのみ使用可能
#ifndef USE_SENSE_INTERRUPT
#define USE_SENSE_INTERRUPT false
#endif

// MatrixScanタスクのスタックサイズ
#ifndef MATRIX_SCAN_TASK_STACK_SIZE
#define MATRIX_SCAN_TASK_STACK_SIZE 128
#endif

// MatrixScanタスクのプライオリティ
#ifndef MATRIX_SCAN_TASK_PRIO
#define MATRIX_SCAN_TASK_PRIO 1
#endif
