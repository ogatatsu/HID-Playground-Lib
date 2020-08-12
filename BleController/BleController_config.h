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

// デバイス情報
#ifndef MANUFACTURER_NAME
#define MANUFACTURER_NAME "..."
#endif

#ifndef MODEL_NUMBER
#define MODEL_NUMBER "..."
#endif

// BLEの送信電波強度: -40, -30, -20, -16, -12, -8, -4, 0, 4
#ifndef TX_POWER
#define TX_POWER 4
#endif

// ペアリングする時にセントラル側の設定画面に現れる見た目
#ifndef BLE_APPEARANCE
#define BLE_APPEARANCE BLE_APPEARANCE_HID_KEYBOARD
#endif

// Minimum guaranteed number of Handle Value Notifications that can be queued for transmission.
#ifndef HVN_TX_QUEUE_SIZE
#define HVN_TX_QUEUE_SIZE 2
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

/*
  According to Accessory Design Guidelines for Apple Devices:

  The connection parameter request may be rejected if it does not complywith all of these rules :

  ● Slave Latency ≤ 30
  ● 2 seconds ≤ connSupervisionTimeout ≤ 6 seconds
  ● Interval Min modulo 15 ms == 0
  ● Interval Min ≥ 15 ms
  ● One of the following :
    ● Interval Min + 15 ms ≤ Interval Max
    ● Interval Min == Interval Max == 15 ms
  ● Interval Max * (Slave Latency + 1) ≤ 2 seconds
  ● Interval Max * (Slave Latency + 1) * 3 < connSupervisionTimeout

  Note that if an accessory requests Interval Min == Interval Max == 15 ms, somedevices will scale the interval
  to 30 ms to balance power and performance constraints.

  If Bluetooth Low Energy HID is one of the connected services of an accessory, connection interval down to
  11 .25 ms may be accepted by the device.
*/

// Connection Interval (unit of 1.25ms)
#ifndef CONNECTION_INTERVAL
#define CONNECTION_INTERVAL 9
#endif

// Slave Latency
#ifndef SLAVE_LATENCY
#define SLAVE_LATENCY 30
#endif

// Supervision Timeout (unit of 10ms)
#ifndef SUPERVISION_TIMEOUT
#define SUPERVISION_TIMEOUT 200
#endif

// スレーブがある場合のみ定義する
// スレーブ側のアドレスのリスト、このリストでフィルタして他の機器と接続しないようにする
// 定義するとBleControllerにセントラル用の関数が生える
// （例）
/*
 #define SLAVE_ADDR_LIST {                 \
     {0x6E, 0x4A, 0x38, 0x76, 0x43, 0xF9}, \
     {0xA6, 0xA2, 0x06, 0xA0, 0xCF, 0xDD}, \
 }
*/

// スレーブがある場合のみ使用される
// スキャン時に点滅させるLEDのピン番号
#ifndef SCAN_LED_PIN
#define SCAN_LED_PIN PIN_LED1
#endif

// スレーブがある場合のみ使用される
// スキャンLEDがHIGHとLOWどちらでONになるか
#ifndef SCAN_LED_ACTIVE_STATE
#define SCAN_LED_ACTIVE_STATE HIGH
#endif
