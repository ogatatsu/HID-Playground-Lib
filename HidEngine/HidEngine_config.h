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

// レイヤーのサイズ
#ifndef LAYER_SIZE
#define LAYER_SIZE 8
#endif

// TapDanceコマンドのタップ判定時間 (ms)
#ifndef TAPPING_TERM_MS
#define TAPPING_TERM_MS 200
#endif

// 同時押しキーマップの最大同時押し数
#ifndef MAX_SIMUL_PRESS_COUNT
#define MAX_SIMUL_PRESS_COUNT 5
#endif

// シーケンスキーマップの最大シーケンス数
#ifndef MAX_SEQ_COUNT
#define MAX_SEQ_COUNT 5
#endif

// MouseMoveコマンドの最初のキープレス時のディレイ
#ifndef MOUSEKEY_DELAY_MS
#define MOUSEKEY_DELAY_MS 200
#endif

// MouseMoveのマウスカーソルの動く間隔 (ms)
// ワイヤレスの場合下げすぎると不安定になるかも
#ifndef MOUSEKEY_INTERVAL_MS
#define MOUSEKEY_INTERVAL_MS 15
#endif

// HidEngineタスクのスタックサイズ
#ifndef HID_ENGINE_TASK_STACK_SIZE
#define HID_ENGINE_TASK_STACK_SIZE 192
#endif

// HidEngine内部で使用しているイベントキューのサイズ
#ifndef HID_ENGINE_EVENT_QUEUE_SIZE
#define HID_ENGINE_EVENT_QUEUE_SIZE 16
#endif
