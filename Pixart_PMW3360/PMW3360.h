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

#include "ThreadSafeSPI.h"

namespace hidpg
{

class PMW3360
{
public:
  enum class Mode
  {
    Run,
    Rest,
  };

  enum class Cpi : uint8_t
  {
    _100   = 0x00, _200   = 0x01, _300   = 0x02,  _400  = 0x03, _500   = 0x04,
    _600   = 0x05, _700   = 0x06, _800   = 0x07,  _900  = 0x08, _1000  = 0x09,
    _1100  = 0x0a, _1200  = 0x0b, _1300  = 0x0c, _1400  = 0x0d, _1500  = 0x0e,
    _1600  = 0x0f, _1700  = 0x10, _1800  = 0x11, _1900  = 0x12, _2000  = 0x13,
    _2100  = 0x14, _2200  = 0x15, _2300  = 0x16, _2400  = 0x17, _2500  = 0x18,
    _2600  = 0x19, _2700  = 0x1a, _2800  = 0x1b, _2900  = 0x1c, _3000  = 0x1d,
    _3100  = 0x1e, _3200  = 0x1f, _3300  = 0x20, _3400  = 0x21, _3500  = 0x22,
    _3600  = 0x23, _3700  = 0x24, _3800  = 0x25, _3900  = 0x26, _4000  = 0x27,
    _4100  = 0x28, _4200  = 0x29, _4300  = 0x2a, _4400  = 0x2b, _4500  = 0x2c,
    _4600  = 0x2d, _4700  = 0x2e, _4800  = 0x2f, _4900  = 0x30, _5000  = 0x31,
    _5100  = 0x32, _5200  = 0x33, _5300  = 0x34, _5400  = 0x35, _5500  = 0x36,
    _5600  = 0x37, _5700  = 0x38, _5800  = 0x39, _5900  = 0x3a, _6000  = 0x3b,
    _6100  = 0x3c, _6200  = 0x3d, _6300  = 0x3e, _6400  = 0x3f, _6500  = 0x40,
    _6600  = 0x41, _6700  = 0x42, _6800  = 0x43, _6900  = 0x44, _7000  = 0x45,
    _7100  = 0x46, _7200  = 0x47, _7300  = 0x48, _7400  = 0x49, _7500  = 0x4a,
    _7600  = 0x4b, _7700  = 0x4c, _7800  = 0x4d, _7900  = 0x4e, _8000  = 0x4f,
    _8100  = 0x50, _8200  = 0x51, _8300  = 0x52, _8400  = 0x53, _8500  = 0x54,
    _8600  = 0x55, _8700  = 0x56, _8800  = 0x57, _8900  = 0x58, _9000  = 0x59,
    _9100  = 0x5a, _9200  = 0x5b, _9300  = 0x5c, _9400  = 0x5d, _9500  = 0x5e,
    _9600  = 0x5f, _9700  = 0x60, _9800  = 0x61, _9900  = 0x62, _10000 = 0x63,
    _10100 = 0x64, _10200 = 0x65, _10300 = 0x66, _10400 = 0x67, _10500 = 0x68,
    _10600 = 0x69, _10700 = 0x6a, _10800 = 0x6b, _10900 = 0x6c, _11000 = 0x6d,
    _11100 = 0x6e, _11200 = 0x6f, _11300 = 0x70, _11400 = 0x71, _11500 = 0x72,
    _11600 = 0x73, _11700 = 0x74, _11800 = 0x75, _11900 = 0x76, _12000 = 0x77,
  };

  using callback_t = void (*)(int16_t deltaX, int16_t deltaY);

  template <uint8_t ID>
  static PMW3360 &create(ThreadSafeSPIClass &spi, uint8_t ncsPin, uint8_t interruptPin)
  {
    static_assert(ID < 2, "Two or more PMW3360 can not be created.");
    if (instances[ID] == nullptr)
    {
      instances[ID] = new PMW3360(spi, ncsPin, interruptPin, ID);
    }
    return *instances[ID];
  }

  template <uint8_t ID>
  static PMW3360 *getInstance()
  {
    static_assert(ID < 2, "Two or more PMW3360 can not be created.");
    return instances[ID];
  }

  void setMotionCallback(callback_t callback);
  void init();
  void startTask();
  void changeMode(Mode mode);
  void changeCpi(Cpi cpi);
  void resetCpi();
  void enableAngleSnap();
  void disableAngleSnap();

#ifdef ARDUINO_ARCH_NRF52
  void stopTask_and_setWakeUpInterrupt();
#endif

private:
  struct MotionBurstData
  {
    union {
      uint8_t raw[12];
      struct
      {
        uint8_t motion;
        uint8_t observation;
        int16_t deltaX;
        int16_t deltaY;
        uint8_t squal;
        uint8_t rawDataSum;
        uint8_t maximumRawData;
        uint8_t minimumRawData;
        uint16_t shutter;
      };
    };
  };

  PMW3360(ThreadSafeSPIClass &spi, uint8_t ncsPin, uint8_t interruptPin, uint8_t id);

  static void task(void *pvParameters);
  static void timeout(TimerHandle_t th);
  static void interrupt_callback_0();
  static void interrupt_callback_1();
  static TaskHandle_t _taskHandles[2];
  static PMW3360 *instances[2];

  void writeRegister(uint8_t addr, uint8_t data);
  uint8_t readRegister(uint8_t addr);
  void readMotionBurst(MotionBurstData &data, uint8_t length);
  void SROM_Download();
  void powerUp();
  void initRegisters();

  TimerHandle_t _timerHandle;
  ThreadSafeSPIClass &_spi;
  const uint8_t _ncsPin;
  const uint8_t _interruptPin;
  const uint8_t _id;
  callback_t _callback;
};

} // namespace hidpg
