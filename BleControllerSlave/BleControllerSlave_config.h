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

// デバイスの名前、ペアリングする時にPCやスマホ側に出てくる名前
#ifndef DEVICE_NAME
#define DEVICE_NAME "..."
#endif

// BLEの送信電波強度: -40, -30, -20, -16, -12, -8, -4, 0, 4
#ifndef TX_POWER
#define TX_POWER 4
#endif

// Minimum guaranteed number of Handle Value Notifications that can be queued for transmission.
#ifndef HVN_TX_QUEUE_SIZE
#define HVN_TX_QUEUE_SIZE 8
#endif

// アドバタイジング時に点滅させるLEDのピン番号
#ifndef ADV_LED_PIN
#define ADV_LED_PIN PIN_LED2
#endif

// アドバタイジングLEDがHIGHとLOWどちらでONになるか
#ifndef ADV_LED_ACTIVE_STATE
#define ADV_LED_ACTIVE_STATE HIGH
#endif

// LEDをHighDriveで光らせるかどうか
#ifndef IS_HIGH_DRIVE
#define IS_HIGH_DRIVE false
#endif

// 自身のアドレスを変更したい場合は定義する(random static address)
// （例）
// #define OWN_ADDR {0x36, 0x9E, 0x59, 0xB6, 0xF4, 0xE0}
