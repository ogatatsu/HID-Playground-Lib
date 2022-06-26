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

#include "PMW3360DM.h"
#include "PMW3360DM_Firmware.h"

namespace hidpg
{
  // registers
  constexpr uint8_t Product_ID = 0x00;
  constexpr uint8_t Revision_ID = 0x01;
  constexpr uint8_t Motion = 0x02;
  constexpr uint8_t Delta_X_L = 0x03;
  constexpr uint8_t Delta_X_H = 0x04;
  constexpr uint8_t Delta_Y_L = 0x05;
  constexpr uint8_t Delta_Y_H = 0x06;
  constexpr uint8_t SQUAL = 0x07;
  constexpr uint8_t Raw_Data_Sum = 0x08;
  constexpr uint8_t Maximum_Raw_data = 0x09;
  constexpr uint8_t Minimum_Raw_data = 0x0A;
  constexpr uint8_t Shutter_Lower = 0x0B;
  constexpr uint8_t Shutter_Upper = 0x0C;
  constexpr uint8_t Control = 0x0D;
  constexpr uint8_t Config1 = 0x0F;
  constexpr uint8_t Config2 = 0x10;
  constexpr uint8_t Angle_Tune = 0x11;
  constexpr uint8_t Frame_Capture = 0x12;
  constexpr uint8_t SROM_Enable = 0x13;
  constexpr uint8_t Run_Downshift = 0x14;
  constexpr uint8_t Rest1_Rate_Lower = 0x15;
  constexpr uint8_t Rest1_Rate_Upper = 0x16;
  constexpr uint8_t Rest1_Downshift = 0x17;
  constexpr uint8_t Rest2_Rate_Lower = 0x18;
  constexpr uint8_t Rest2_Rate_Upper = 0x19;
  constexpr uint8_t Rest2_Downshift = 0x1A;
  constexpr uint8_t Rest3_Rate_Lower = 0x1B;
  constexpr uint8_t Rest3_Rate_Upper = 0x1C;
  constexpr uint8_t Observation = 0x24;
  constexpr uint8_t Data_Out_Lower = 0x25;
  constexpr uint8_t Data_Out_Upper = 0x26;
  constexpr uint8_t Raw_Data_Dump = 0x29;
  constexpr uint8_t SROM_ID = 0x2A;
  constexpr uint8_t Min_SQ_Run = 0x2B;
  constexpr uint8_t Raw_Data_Threshold = 0x2C;
  constexpr uint8_t Config5 = 0x2F;
  constexpr uint8_t Power_Up_Reset = 0x3A;
  constexpr uint8_t Shutdown = 0x3B;
  constexpr uint8_t Inverse_Product_ID = 0x3F;
  constexpr uint8_t LiftCutoff_Tune3 = 0x41;
  constexpr uint8_t Angle_Snap = 0x42;
  constexpr uint8_t LiftCutoff_Tune1 = 0x4A;
  constexpr uint8_t Motion_Burst = 0x50;
  constexpr uint8_t LiftCutoff_Tune_Timeout = 0x58;
  constexpr uint8_t LiftCutoff_Tune_Min_Length = 0x5A;
  constexpr uint8_t SROM_Load_Burst = 0x62;
  constexpr uint8_t Lift_Config = 0x63;
  constexpr uint8_t Raw_Data_Burst = 0x64;
  constexpr uint8_t LiftCutoff_Tune2 = 0x65;

  // spi parameter
  static SPISettings SpiSettings(2000000, MSBFIRST, SPI_MODE3);

  PMW3360DM::PMW3360DM(ThreadSafeSPIClass &spi,
                       uint8_t ncs_pin,
                       uint8_t interrupt_pin,
                       TaskHandle_t &task_handle,
                       voidFuncPtr interrupt_callback)
      : _spi(spi),
        _ncs_pin(ncs_pin),
        _interrupt_pin(interrupt_pin),
        _task_handle(task_handle),
        _interrupt_callback(interrupt_callback),
        _callback(nullptr)
  {
  }

  void PMW3360DM::setCallback(callback_t callback)
  {
    _callback = callback;
  }

  void PMW3360DM::start()
  {
    _spi.begin();
    _spi.usingInterrupt(_interrupt_pin);

    pinMode(_ncs_pin, OUTPUT);
    pinMode(_interrupt_pin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(_interrupt_pin), _interrupt_callback, FALLING);

    _mutex = xSemaphoreCreateMutexStatic(&_mutex_buffer);
    _task_handle = xTaskCreateStatic(task, "3360", PMW3360DM_TASK_STACK_SIZE, this, PMW3360DM_TASK_PRIO, _task_stack, &_task_tcb);

    powerUp();
  }

  void PMW3360DM::task(void *pvParameters)
  {
    PMW3360DM *that = static_cast<PMW3360DM *>(pvParameters);

    while (true)
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if (that->_callback != nullptr)
      {
        that->_callback();
      }
    }
  }

  void PMW3360DM::writeRegister(uint8_t addr, uint8_t data)
  {
    _spi.beginTransaction(SpiSettings);
    digitalWrite(_ncs_pin, LOW);

    delayMicroseconds(1); // tNCS-SCLK: 120ns

    // The first byte contains the address (seven bits) and has a “1” as its MSB to indicate data direction.
    // The second byte contains the data.
    _spi.transfer16(((addr | 0b10000000) << 8) | data);

    delayMicroseconds(35); // tSCLK-NCS(Write): 35us

    digitalWrite(_ncs_pin, HIGH);
    _spi.endTransaction();

    delayMicroseconds(145); // (tSWW/tSWR: 180us) - (tSCLK-NCS: 35us) = 145us
  }

  uint8_t PMW3360DM::readRegister(uint8_t addr)
  {
    _spi.beginTransaction(SpiSettings);
    digitalWrite(_ncs_pin, LOW);

    delayMicroseconds(1); // tNCS-SCLK (120ns)

    // The first byte contains the address, is sent by the micro‐controller over MOSI, and has a “0” as its MSB to indicate data direction.
    _spi.transfer(addr & 0b01111111);
    delayMicroseconds(160); // tSRAD (160us)
    // read data
    uint8_t data = _spi.transfer(0);

    delayMicroseconds(1); // tSCLK-NCS(Read): 120ns

    digitalWrite(_ncs_pin, HIGH);
    _spi.endTransaction();

    delayMicroseconds(19); // (tSRW/tSRR: 20us) - (tSCLK-NCS: 35us) = 19us

    return data;
  }

  void PMW3360DM::readMotionBurst(MotionBurstData *data, uint8_t length)
  {
    length = min(static_cast<uint8_t>(12), length);

    // 1.Write any value to Motion_Burst register.
    writeRegister(Motion_Burst, 0);

    // 2.Lower NCS
    _spi.beginTransaction(SpiSettings);
    digitalWrite(_ncs_pin, LOW);

    // 3.Send Motion_Burst address (0x50).
    _spi.transfer(0x50);

    // 4.Wait for tSRAD_MOTBR: 35us
    delayMicroseconds(35);

    // 5.Start reading SPI Data continuously up to 12 bytes. Motion burst may be terminated by pulling NCS high for at least tBEXIT.
    _spi.transfer(data->raw, length);

    digitalWrite(_ncs_pin, HIGH);
    _spi.endTransaction();

    delayMicroseconds(1); // tBEXIT: 500ns

    // 6.To read new motion burst data, repeat from step 2.
    // 7.If a non‐burst register read operation was executed; then, to read new burst data, start from step 1 instead.
  }

  void PMW3360DM::SROM_Download()
  {
    // 1.Perform the Power‐Up sequence

    // 2.Write 0 to Rest_En bit of Config2 register to disable Rest mode.
    writeRegister(Config2, 0);

    // 3.Write 0x1d to SROM_Enable register for initializing
    writeRegister(SROM_Enable, 0x1d);

    // 4.Wait for 10 ms
    delay(10);

    // 5.Write 0x18 to SROM_Enable register again to start SROM Download
    writeRegister(SROM_Enable, 0x18);

    // 6.Write SROM file into SROM_Load_Burst register, 1st data must start with SROM_Load_Burst address.
    // All the SROM data must be downloaded before SROM starts running.
    _spi.beginTransaction(SpiSettings);
    digitalWrite(_ncs_pin, LOW);

    _spi.transfer(SROM_Load_Burst | 0b10000000);
    delayMicroseconds(15);

    // send all bytes of the firmware
    for (size_t i = 0; i < sizeof(PMW3360DM_Firmware); i++)
    {
      _spi.transfer(PMW3360DM_Firmware[i]);
      delayMicroseconds(15);
    }

    _spi.endTransaction();
    digitalWrite(_ncs_pin, HIGH); // exit burst mode
    delayMicroseconds(185);       // 200us-15us

    // 7.Read the SROM_ID register to verify the ID before any other register reads or writes.
    readRegister(SROM_ID);

    // 8.Write 0x00 to Config2 register for wired mouse or 0x20 for wireless mouse design.
    writeRegister(Config2, 0x20);
  }

  void PMW3360DM::powerUp()
  {
    // 1.Apply power to VDD and VDDIO in any order, with a delay of no more than 100ms in between each supply. Ensure all supplies are stable.

    // 2.Drive NCS high, and then low to reset the SPI port.
    digitalWrite(_ncs_pin, HIGH);
    digitalWrite(_ncs_pin, LOW);
    // 3.Write 0x5A to Power_Up_Reset register (or, alternatively toggle the NRESET pin).
    writeRegister(Power_Up_Reset, 0x5a);
    // 4.Wait for at least 50ms.
    delay(50);
    // 5.Read from registers 0x02, 0x03, 0x04, 0x05 and 0x06 one time regardless of the motion pin state.
    readRegister(Motion);
    readRegister(Delta_X_L);
    readRegister(Delta_X_H);
    readRegister(Delta_Y_L);
    readRegister(Delta_Y_H);
    // 6.Perform SROM download.
    SROM_Download();

    // 7.Load configuration for other registers.
    initRegisters();
  }

  void PMW3360DM::initRegisters()
  {
    writeRegister(Control, PMW3360DM_Control);
    writeRegister(Angle_Tune, PMW3360DM_Angle_Tune);

    writeRegister(Run_Downshift, PMW3360DM_Run_Downshift);
    writeRegister(Rest1_Rate_Lower, PMW3360DM_Rest1_Rate & 0b0000000011111111);
    writeRegister(Rest1_Rate_Upper, (PMW3360DM_Rest1_Rate & 0b1111111100000000) >> 8);

    writeRegister(Rest1_Downshift, PMW3360DM_Rest1_Downshift);
    writeRegister(Rest2_Rate_Lower, PMW3360DM_Rest2_Rate & 0b0000000011111111);
    writeRegister(Rest2_Rate_Upper, (PMW3360DM_Rest2_Rate & 0b1111111100000000) >> 8);

    writeRegister(Rest2_Downshift, PMW3360DM_Rest2_Downshift);
    writeRegister(Rest3_Rate_Lower, PMW3360DM_Rest3_Rate & 0b0000000011111111);
    writeRegister(Rest3_Rate_Upper, (PMW3360DM_Rest3_Rate & 0b1111111100000000) >> 8);

    writeRegister(Lift_Config, PMW3360DM_Lift_Config);
  }

  void PMW3360DM::readDelta(int16_t *delta_x, int16_t *delta_y)
  {
    MotionBurstData mb_data;
    xSemaphoreTake(_mutex, portMAX_DELAY);
    readMotionBurst(&mb_data, 6);
    xSemaphoreGive(_mutex);
    *delta_x = mb_data.delta_x;
    *delta_y = mb_data.delta_y;
  }

  void PMW3360DM::changeMode(Mode mode)
  {
    uint8_t data = (mode == Mode::Run) ? 0b00000000 : 0b00100000;
    xSemaphoreTake(_mutex, portMAX_DELAY);
    writeRegister(Config2, data);
    xSemaphoreGive(_mutex);
  }

  void PMW3360DM::changeCpi(Cpi cpi)
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    writeRegister(Config1, static_cast<uint8_t>(cpi));
    xSemaphoreGive(_mutex);
  }

  void PMW3360DM::enableAngleSnap()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    writeRegister(Angle_Snap, 0b10000000);
    xSemaphoreGive(_mutex);
  }

  void PMW3360DM::disableAngleSnap()
  {
    xSemaphoreTake(_mutex, portMAX_DELAY);
    writeRegister(Angle_Snap, 0b00000000);
    xSemaphoreGive(_mutex);
  }

#ifdef ARDUINO_ARCH_NRF52
  void PMW3360DM::stop_and_setWakeUpInterrupt()
  {
    vTaskSuspend(_task_handle);

    NRF_GPIO->PIN_CNF[_interrupt_pin] |= ((uint32_t)(GPIO_PIN_CNF_SENSE_Low) << GPIO_PIN_CNF_SENSE_Pos);
  }
#endif

} // namespace hidpg
