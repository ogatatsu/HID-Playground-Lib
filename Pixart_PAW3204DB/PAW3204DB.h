/*
  The MIT License (MIT)

  Copyright (c) 2020 ogatatsu.

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

#include "FreeRTOS.h"

#ifdef ARDUINO_ARCH_NRF52
#include "PAW3204DB_RegOperator_nRF52.h"
#else
#include "PAW3204DB_RegOperator_GPIO.h"
#endif

namespace hidpg
{

  class PAW3204DB
  {
  public:
    enum class Mode : uint8_t
    {
      Run = 0b10100001,
      Sleep1 = 0b10110010,
      Sleep2 = 0b10111100,
    };

    enum class Cpi : uint8_t
    {
      _400 = 0b000,
      _500 = 0b001,
      _600 = 0b010,
      _800 = 0b011,
      _1000 = 0b100,
      _1200 = 0b101,
      _1600 = 0b110,
    };

    using callback_t = void (*)(void);

    template <uint8_t ID>
    static PAW3204DB &create(uint8_t sclk_pin, uint8_t sdio_pin, uint8_t motswk_pin)
    {
      static_assert(ID < 2, "Two or more PAW3204DB can not be created.");
      if (instances[ID] == nullptr)
      {
        PAW3204DB_RegOperator *reg;
#ifdef ARDUINO_ARCH_NRF52
        if (ID == 0)
          reg = new PAW3204DB_RegOperator_nRF52(NRF_SPI2, sclk_pin, sdio_pin);
        else
          reg = new PAW3204DB_RegOperator_nRF52(NRF_SPI1, sclk_pin, sdio_pin);
#else
        reg = new PAW3204DB_RegOperator_GPIO(sclk_pin, sdio_pin);
#endif
        instances[ID] = new PAW3204DB(reg, motswk_pin, ID);
      }
      return *instances[ID];
    }

    template <uint8_t ID>
    static PAW3204DB *getInstance()
    {
      static_assert(ID < 2, "Two or more PAW3204DB can not be created.");
      return instances[ID];
    }

    void setCallback(callback_t callback);
    void start();
    void readDelta(int16_t *delta_x, int16_t *delta_y);
    void changeCpi(Cpi cpi);
    void changeMode(Mode mode);

#ifdef ARDUINO_ARCH_NRF52
    void stop_and_setWakeUpInterrupt();
#endif

  private:
    PAW3204DB(PAW3204DB_RegOperator *reg, uint8_t motswk_pin, uint8_t id);

    static void task(void *pvParameters);
    static void timer_callback(TimerHandle_t timer_handle);
    static void interrupt_callback_0();
    static void interrupt_callback_1();
    static TaskHandle_t _task_handles[2];
    static PAW3204DB *instances[2];

    void initRegisters();

    SemaphoreHandle_t _mutex;
    PAW3204DB_RegOperator *_reg;
    const uint8_t _motswk_pin;
    const uint8_t _id;
    callback_t _callback;
  };

} // namespace hidpg
