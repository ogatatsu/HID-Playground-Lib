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
#ifndef BLE_DEVICE_NAME
#define BLE_DEVICE_NAME "..."
#endif

// BLEの送信電波強度: -40, -30, -20, -16, -12, -8, -4, 0, 4
#ifndef BLE_TX_POWER
#define BLE_TX_POWER 4
#endif

// Minimum guaranteed number of Handle Value Notifications that can be queued for transmission.
#ifndef BLE_HVN_TX_QUEUE_SIZE
#define BLE_HVN_TX_QUEUE_SIZE 2
#endif

// アドバタイジング時に点滅させるLEDのピン番号
#ifndef BLE_ADV_LED_PIN
#define BLE_ADV_LED_PIN PIN_LED2
#endif

// アドバタイジングLEDがHIGHとLOWどちらでONになるか
#ifndef BLE_ADV_LED_ACTIVE_STATE
#define BLE_ADV_LED_ACTIVE_STATE HIGH
#endif

// 自身のアドレスを変更したい場合は定義する(random static address)
// （例）
// #define BLE_OWN_ADDR {0x36, 0x9E, 0x59, 0xB6, 0xF4, 0xE0}

// SUPERVISION_TIMEOUT * 4 > (1 + SLAVE_LATENCY) * CONNECTION_INTERVAL
// Connection Interval (unit of 1.25ms)
#ifndef BLE_CONNECTION_INTERVAL
#define BLE_CONNECTION_INTERVAL 6
#endif

// Slave Latency
#ifndef BLE_SLAVE_LATENCY
#define BLE_SLAVE_LATENCY 65
#endif

// Supervision Timeout (unit of 10ms)
#ifndef BLE_SUPERVISION_TIMEOUT
#define BLE_SUPERVISION_TIMEOUT 100
#endif
