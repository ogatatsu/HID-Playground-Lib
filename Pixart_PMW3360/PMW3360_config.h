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

// Control register(0x0D)
//   0 degree: 0b00000000
//  90 degree: 0b11000000
// 180 degree: 0b01100000
// 270 degree: 0b10100000
#ifndef PMW3360_Control
#define PMW3360_Control 0b00000000
#endif

// Config1 register(0x0F)
// 0x00: 100cpi
// 0x01: 200cpi
// ~
// 0x77: 12000cpi
#ifndef PMW3360_Config1
#define PMW3360_Config1 0x31 // 5000cpi
#endif

// Angle_Tune register(0x11)
// -30 degree: 0xE2
// -10 degree: 0xF6
//   0 degree: 0x00
// +10 degree: 0x0F
// +30 degree: 0x1E
#ifndef PMW3360_Angle_Tune
#define PMW3360_Angle_Tune 0x00
#endif

// Run_Downshift register(0x14)
// Run Downshift time(ms) = RD([7:0]) x 10 ms
#ifndef PMW3360_Run_Downshift
#define PMW3360_Run_Downshift 50 // 500ms
#endif

// Rest1_Rate_Lower register(0x15), Rest1_Rate_Upper register(0x16)
// R1R[15:0] value must not exceed 0x09B0, otherwise an internal watchdog will trigger a reset.
// Rest1 frame rate duration = (R1R[15:0] + 1) x 1 ms
#ifndef PMW3360_Rest1_Rate
#define PMW3360_Rest1_Rate 0 // 1ms
#endif

// Rest1_Downshift register(0x17)
// Rest1 Downshift time = R1D[7:0] x 320 x Rest1_Rate.
#ifndef PMW3360_Rest1_Downshift
#define PMW3360_Rest1_Downshift 31 // 9.92s
#endif

// Rest2_Rate_Lower(0x18), Rest2_Rate_Upper(0x19)
// R2R[15:0] value must not exceed 0x09B0, otherwise an internal watchdog will trigger a reset.
// Rest2 frame rate duration = (R2R[15:0] + 1) x 1 ms
#ifndef PMW3360_Rest2_Rate
#define PMW3360_Rest2_Rate 99 // 100ms
#endif

// Rest2_Downshift(0x1A)
// Rest2 Downshift time = R2D[7:0] x 32 x Rest2_Rate.
#ifndef PMW3360_Rest2_Downshift
#define PMW3360_Rest2_Downshift 188 // 601.6s = 10min
#endif

// Rest3_Rate_Lower(0x1B), Rest3_Rate_Upper(0x1C)
// R3R[15:0] value must not exceed 0x09B0, otherwise an internal watchdog will trigger a reset.
// Rest3 frame rate duration = (R3R[15:0] + 1) x 1 ms
#ifndef PMW3360_Rest3_Rate
#define PMW3360_Rest3_Rate 499 // 500ms
#endif

// Lift_Config register(0x63)
// 2mm: 0b10
// 3mm: 0b11
#ifndef PMW3360_Lift_Config
#define PMW3360_Lift_Config 0b10
#endif

// 各モードでのloopTaskにデータを送る間隔(ms)
#ifndef PMW3360_RUN_MODE_CALLBACK_INTERVAL
#define PMW3360_RUN_MODE_CALLBACK_INTERVAL 1
#endif

#ifndef PMW3360_REST_MODE_CALLBACK_INTERVAL
#define PMW3360_REST_MODE_CALLBACK_INTERVAL 10
#endif

// PMW3360タスクのスタックサイズ
#ifndef PMW3360_TASK_STACK_SIZE
#define PMW3360_TASK_STACK_SIZE 128
#endif

// PMW3360タスクのプライオリティ
#ifndef PMW3360_TASK_PRIO
#define PMW3360_TASK_PRIO TASK_PRIO_LOW
#endif
