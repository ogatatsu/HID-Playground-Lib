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

  // event flag bit
  constexpr uint32_t InterruptEventBit = 0;
  constexpr uint32_t TimerEventBit = 1;

  //------------------------------------------------------------------+
  // static member
  //------------------------------------------------------------------+
  TaskHandle_t PAW3204DB::_task_handles[2] = {nullptr, nullptr};
  PAW3204DB *PAW3204DB::instances[2] = {nullptr, nullptr};

  void PAW3204DB::interrupt_callback_0()
  {
    if (_task_handles[0] != nullptr)
    {
      xTaskNotifyFromISR(_task_handles[0], bit(InterruptEventBit), eSetBits, nullptr);
    }
  }

  void PAW3204DB::interrupt_callback_1()
  {
    if (_task_handles[1] != nullptr)
    {
      xTaskNotifyFromISR(_task_handles[1], bit(InterruptEventBit), eSetBits, nullptr);
    }
  }

  void PAW3204DB::task(void *pvParameters)
  {
    PAW3204DB *that = static_cast<PAW3204DB *>(pvParameters);

    int32_t total_delta_x = 0;
    int32_t total_delta_y = 0;
    bool is_motion_active = false;
    bool is_timer_active = false;
    uint32_t event = 0;

    while (true)
    {
      is_motion_active = digitalRead(that->_motswk_pin) == LOW;
      if (is_motion_active || is_timer_active)
      {
        event = ulTaskNotifyTake(pdTRUE, 1);
      }
      else
      {
        event = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      }

      if (bitRead(event, InterruptEventBit) || is_motion_active)
      {
        uint8_t status;
        do
        {
          xSemaphoreTake(that->_mutex, portMAX_DELAY);
          status = that->_reg.read(Motion_Status);
          total_delta_x += static_cast<int8_t>(that->_reg.read(Delta_X));
          total_delta_y += static_cast<int8_t>(that->_reg.read(Delta_Y));
          xSemaphoreGive(that->_mutex);

          // Repeat until the motion occurrence bit becomes 0.
        } while (bitRead(status, 7));

        if (is_timer_active == false)
        {
          xTimerStart(that->_timer_handle, portMAX_DELAY);
          is_timer_active = true;
        }
      }

      if (bitRead(event, TimerEventBit))
      {
        int16_t delta_x = constrain(total_delta_x, INT16_MIN, INT16_MAX);
        total_delta_x -= delta_x;

        int16_t delta_y = constrain(total_delta_y, INT16_MIN, INT16_MAX);
        total_delta_y -= delta_y;

        if ((that->_callback != nullptr) && (delta_x != 0 || delta_y != 0))
        {
          that->_callback(delta_x, delta_y);
        }

        if (total_delta_x == 0 && total_delta_y == 0)
        {
          xTimerStop(that->_timer_handle, portMAX_DELAY);
          is_timer_active = false;
        }
      }
    }
  }

  void PAW3204DB::timer_callback(TimerHandle_t timer_handle)
  {
    PAW3204DB *that = static_cast<PAW3204DB *>(pvTimerGetTimerID(timer_handle));
    xTaskNotify(_task_handles[that->_id], bit(TimerEventBit), eSetBits);
  }

  //------------------------------------------------------------------+
  // instance member
  //------------------------------------------------------------------+
  PAW3204DB::PAW3204DB(uint8_t sclk_pin, uint8_t sdio_pin, uint8_t motswk_pin, uint8_t id)
      : _reg(sclk_pin, sdio_pin), _motswk_pin(motswk_pin), _id(id), _callback(nullptr)
  {
  }

  void PAW3204DB::setCallback(callback_t callback)
  {
    _callback = callback;
  }

  void PAW3204DB::init()
  {
    _reg.init();

    pinMode(_motswk_pin, INPUT_PULLUP);
    void (*interrupt_callback)() = (_id == 0) ? interrupt_callback_0 : interrupt_callback_1;
    attachInterrupt(digitalPinToInterrupt(_motswk_pin), interrupt_callback, FALLING);

    _reg.reSyncSerial();
    initRegisters();

    _mutex = xSemaphoreCreateMutex();
    _timer_handle = xTimerCreate(nullptr, pdMS_TO_TICKS(PAW3204DB_CALLBACK_INTERVAL_MS), true, this, timer_callback);
  }

  void PAW3204DB::startTask()
  {
    char name[] = "3204_0";
    name[5] += _id;
    xTaskCreate(task, name, PAW3204DB_TASK_STACK_SIZE, this, PAW3204DB_TASK_PRIO, &_task_handles[_id]);
  }

  void PAW3204DB::initRegisters()
  {
  }

  void PAW3204DB::changeCpi(Cpi cpi)
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    _reg.write(Configuration, static_cast<uint8_t>(cpi));
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
