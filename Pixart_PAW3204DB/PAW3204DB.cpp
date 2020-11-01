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

#include "Arduino.h"

#include "PAW3204DB.h"
#include "PAW3204DB_config.h"

namespace hidpg
{
  // registers
  constexpr uint8_t Product_ID1 = 0x00;
  constexpr uint8_t Product_ID2 = 0x01;
  constexpr uint8_t Motion_Status = 0x02;
  constexpr uint8_t Delta_X = 0x03;
  constexpr uint8_t Delta_Y = 0x04;
  constexpr uint8_t Operation_Mode = 0x05;
  constexpr uint8_t Configuration = 0x06;
  constexpr uint8_t Image_Quality = 0x07;
  constexpr uint8_t Operation_State = 0x08;
  constexpr uint8_t Write_Protect = 0x09;
  constexpr uint8_t Sleep1_Setting = 0x0A;
  constexpr uint8_t Enter_Time = 0x0B;
  constexpr uint8_t Sleep2_Setting = 0x0C;
  constexpr uint8_t Image_Threshold = 0x0D;
  constexpr uint8_t Image_Recognition = 0x0E;

  //------------------------------------------------------------------+
  // static member
  //------------------------------------------------------------------+
  TaskHandle_t PAW3204DB::_task_handles[2] = {nullptr, nullptr};
  PAW3204DB *PAW3204DB::instances[2] = {nullptr, nullptr};

  void PAW3204DB::interrupt_callback_0()
  {
    if (_task_handles[0] != nullptr)
    {
      xTaskNotifyFromISR(_task_handles[0], 1, eSetValueWithOverwrite, nullptr);
    }
  }

  void PAW3204DB::interrupt_callback_1()
  {
    if (_task_handles[1] != nullptr)
    {
      xTaskNotifyFromISR(_task_handles[1], 1, eSetValueWithOverwrite, nullptr);
    }
  }

  void PAW3204DB::task(void *pvParameters)
  {
    PAW3204DB *that = static_cast<PAW3204DB *>(pvParameters);

    // reset motion pin
    int16_t x, y;
    that->readDelta(&x, &y);

    while (true)
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if (that->_callback != nullptr)
      {
        that->_callback();
      }
    }
  }

  //------------------------------------------------------------------+
  // instance member
  //------------------------------------------------------------------+
  PAW3204DB::PAW3204DB(PAW3204DB_RegOperator *reg, uint8_t motswk_pin, uint8_t id)
      : _reg(reg), _motswk_pin(motswk_pin), _id(id), _callback(nullptr)
  {
  }

  void PAW3204DB::setCallback(callback_t callback)
  {
    _callback = callback;
  }

  void PAW3204DB::begin()
  {
    _mutex = xSemaphoreCreateMutex();

    _reg->begin();

    pinMode(_motswk_pin, INPUT_PULLUP);
    void (*interrupt_callback)() = (_id == 0) ? interrupt_callback_0 : interrupt_callback_1;
    attachInterrupt(digitalPinToInterrupt(_motswk_pin), interrupt_callback, FALLING);

    _reg->reSyncSerial();
    initRegisters();

    char name[] = "3204_0";
    name[5] += _id;
    xTaskCreate(task, name, PAW3204DB_TASK_STACK_SIZE, this, PAW3204DB_TASK_PRIO, &_task_handles[_id]);
  }

  void PAW3204DB::initRegisters()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _reg->write(Write_Protect, static_cast<uint8_t>(0x5A));

    _reg->write(Sleep1_Setting, static_cast<uint8_t>(PAW3204DB_Sleep1_Setting));
    _reg->write(Enter_Time, static_cast<uint8_t>(PAW3204DB_Enter_Time));
    _reg->write(Sleep2_Setting, static_cast<uint8_t>(PAW3204DB_Sleep2_Setting));
    _reg->write(Image_Threshold, static_cast<uint8_t>(PAW3204DB_Image_Threshold));
    _reg->write(Image_Recognition, static_cast<uint8_t>(PAW3204DB_Image_Recognition));

    _reg->write(Write_Protect, static_cast<uint8_t>(0x00));
    xSemaphoreGive(_mutex);
  }

  void PAW3204DB::readDelta(int16_t *delta_x, int16_t *delta_y)
  {
    int16_t total_delta_x = 0;
    int16_t total_delta_y = 0;

    xSemaphoreTake(_mutex, portMAX_DELAY);
    do
    {
      _reg->read(Motion_Status);
      total_delta_x += static_cast<int8_t>(_reg->read(Delta_X));
      total_delta_y += static_cast<int8_t>(_reg->read(Delta_Y));

    } while (digitalRead(_motswk_pin) == LOW);
    xSemaphoreGive(_mutex);

    *delta_x = total_delta_x;
    *delta_y = total_delta_y;
  }

  void PAW3204DB::changeCpi(Cpi cpi)
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _reg->write(Configuration, static_cast<uint8_t>(cpi));
    xSemaphoreGive(_mutex);
  }

  void PAW3204DB::changeMode(Mode mode)
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _reg->write(Operation_Mode, static_cast<uint8_t>(mode));
    xSemaphoreGive(_mutex);
  }

#ifdef ARDUINO_ARCH_NRF52
  void PAW3204DB::stopTask_and_setWakeUpInterrupt()
  {
    vTaskSuspend(_task_handles[_id]);

    NRF_GPIO->PIN_CNF[_motswk_pin] |= ((uint32_t)(GPIO_PIN_CNF_SENSE_Low) << GPIO_PIN_CNF_SENSE_Pos);
  }
#endif

} // namespace hidpg
