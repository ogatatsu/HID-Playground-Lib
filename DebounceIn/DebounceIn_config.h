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

#ifndef DEBOUNCE_IN_MAX_PIN_COUNT
#define DEBOUNCE_IN_MAX_PIN_COUNT 8
#endif

// 割り込みにSense signalを使用するか
// nRF52でのみ使用可能
// Sense signalを使用する場合割り込み数に制限はないのでDEBOUNCE_IN_MAX_PIN_COUNTは無視する
#ifndef DEBOUNCE_IN_USE_SENSE_INTERRUPT
#define DEBOUNCE_IN_USE_SENSE_INTERRUPT false
#endif

// タスクのスタックサイズ
#ifndef DEBOUNCE_IN_TASK_STACK_SIZE
#define DEBOUNCE_IN_TASK_STACK_SIZE 128
#endif

// タスクのプライオリティ
#ifndef DEBOUNCE_IN_TASK_PRIO
#define DEBOUNCE_IN_TASK_PRIO 1
#endif
