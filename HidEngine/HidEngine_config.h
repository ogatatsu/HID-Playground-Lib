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
#ifndef HID_ENGINE_LAYER_SIZE
#define HID_ENGINE_LAYER_SIZE 4
#endif

// タップのスピード
#ifndef HID_ENGINE_TAP_SPEED_MS
#define HID_ENGINE_TAP_SPEED_MS 1
#endif

// CommandTapper内部で使われているキューの最大サイズ
#ifndef HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE
#define HID_ENGINE_COMMAND_TAPPER_QUEUE_SIZE 32
#endif

// TimerMixinで使用できる最大タイマー数
#ifndef HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT
#define HID_ENGINE_TIMER_MIXIN_MAX_TIMER_COUNT 8
#endif

// TapDanceコマンドのタップ判定時間 (ms)
#ifndef HID_ENGINE_TAPPING_TERM_MS
#define HID_ENGINE_TAPPING_TERM_MS 250
#endif

// シーケンスキーマップの最大シーケンス数
#ifndef HID_ENGINE_MAX_SEQ_COUNT
#define HID_ENGINE_MAX_SEQ_COUNT 5
#endif

// MouseMoveコマンドの最初のキープレス時のディレイ
#ifndef HID_ENGINE_MOUSEKEY_DELAY_MS
#define HID_ENGINE_MOUSEKEY_DELAY_MS 200
#endif

// MouseMoveのマウスカーソルの動く間隔 (ms)
// ワイヤレスの場合下げすぎると不安定になるかも
#ifndef HID_ENGINE_MOUSEKEY_INTERVAL_MS
#define HID_ENGINE_MOUSEKEY_INTERVAL_MS 25
#endif

// HidEngineタスクのスタックサイズ
#ifndef HID_ENGINE_TASK_STACK_SIZE
#define HID_ENGINE_TASK_STACK_SIZE 256
#endif

// タスクのプライオリティ
#ifndef HID_ENGINE_TASK_PRIO
#define HID_ENGINE_TASK_PRIO 1
#endif

// HidEngine内部で使用しているイベントキューのサイズ
#ifndef HID_ENGINE_EVENT_QUEUE_SIZE
#define HID_ENGINE_EVENT_QUEUE_SIZE 8
#endif
