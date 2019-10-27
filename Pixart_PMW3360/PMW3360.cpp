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

#include "PMW3360.h"
#include "PMW3360_Firmware.h"
#include "PMW3360_config.h"

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

// event flag bit
constexpr uint32_t InterruptEventBit = 0;
constexpr uint32_t TimerEventBit = 1;

// spi setting
static SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE3);

/*------------------------------------------------------------------*/
/* static member
 *------------------------------------------------------------------*/
TaskHandle_t PMW3360::_taskHandles[2] = {nullptr, nullptr};
PMW3360 *PMW3360::instances[2] = {nullptr, nullptr};

void PMW3360::interrupt_callback_0()
{
  if (_taskHandles[0] != nullptr)
  {
    xTaskNotifyFromISR(_taskHandles[0], bit(InterruptEventBit), eSetBits, nullptr);
  }
}

void PMW3360::interrupt_callback_1()
{
  if (_taskHandles[1] != nullptr)
  {
    xTaskNotifyFromISR(_taskHandles[1], bit(InterruptEventBit), eSetBits, nullptr);
  }
}

void PMW3360::task(void *pvParameters)
{
  PMW3360 *that = static_cast<PMW3360 *>(pvParameters);

  int32_t totalDeltaX = 0;
  int32_t totalDeltaY = 0;
  bool isTimerActive = false;

  while (true)
  {
    uint32_t event = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (bitRead(event, InterruptEventBit))
    {
      MotionBurstData mbdata;
      that->readMotionBurst(mbdata, 6);

      totalDeltaX += mbdata.deltaX;
      totalDeltaY += mbdata.deltaY;

      if (isTimerActive == false)
      {
        xTimerStart(that->_timerHandle, portMAX_DELAY);
        isTimerActive = true;
      }
    }

    if (bitRead(event, TimerEventBit))
    {
      int16_t deltaX = constrain(totalDeltaX, INT16_MIN, INT16_MAX);
      totalDeltaX -= deltaX;

      int16_t deltaY = constrain(totalDeltaY, INT16_MIN, INT16_MAX);
      totalDeltaY -= deltaY;

      if (that->_callback != nullptr)
      {
        that->_callback(deltaX, deltaY);
      }

      if (totalDeltaX == 0 && totalDeltaY == 0)
      {
        xTimerStop(that->_timerHandle, portMAX_DELAY);
        isTimerActive = false;
      }
    }
  }
}

void PMW3360::timeout(TimerHandle_t th)
{
  PMW3360 *that = static_cast<PMW3360 *>(pvTimerGetTimerID(th));
  xTaskNotify(_taskHandles[that->_id], bit(TimerEventBit), eSetBits);
}

/*------------------------------------------------------------------*/
/* instance member
 *------------------------------------------------------------------*/
PMW3360::PMW3360(ThreadSafeSPIClass &spi, uint8_t ncsPin, uint8_t interruptPin, uint8_t id)
    : _spi(spi), _ncsPin(ncsPin), _interruptPin(interruptPin), _id(id), _callback(nullptr)
{
}

void PMW3360::setCallback(callback_t callback)
{
  _callback = callback;
}

void PMW3360::init()
{
  _spi.begin();
  _spi.usingInterrupt(_interruptPin);

  pinMode(_ncsPin, OUTPUT);
  pinMode(_interruptPin, INPUT_PULLUP);

  void (*interrupt_callback)() = (_id == 0) ? interrupt_callback_0 : interrupt_callback_1;
  attachInterrupt(digitalPinToInterrupt(_interruptPin), interrupt_callback, FALLING);

  // デフォルトはRest mode
  _timerHandle = xTimerCreate(nullptr, pdMS_TO_TICKS(PMW3360_REST_MODE_CALLBACK_INTERVAL), true, this, timeout);
}

void PMW3360::startTask()
{
  char name[] = "3360_0";
  name[5] += _id;
  xTaskCreate(task, name, PMW3360_TASK_STACK_SIZE, this, PMW3360_TASK_PRIO, &_taskHandles[_id]);

  powerUp();
}

void PMW3360::writeRegister(uint8_t addr, uint8_t data)
{
  _spi.beginTransaction(spiSettings);
  digitalWrite(_ncsPin, LOW);

  delayMicroseconds(1); // tNCS-SCLK: 120ns

  // The first byte contains the address (seven bits) and has a “1” as its MSB to indicate data direction.
  // The second byte contains the data.
  _spi.transfer16(((addr | 0b10000000) << 8) | data);

  delayMicroseconds(35); // tSCLK-NCS(Write): 35us

  digitalWrite(_ncsPin, HIGH);
  _spi.endTransaction();

  delayMicroseconds(145); // (tSWW/tSWR: 180us) - (tSCLK-NCS: 35us) = 145us
}

uint8_t PMW3360::readRegister(uint8_t addr)
{
  _spi.beginTransaction(spiSettings);
  digitalWrite(_ncsPin, LOW);

  delayMicroseconds(1); // tNCS-SCLK (120ns)

  // The first byte contains the address, is sent by the micro‐controller over MOSI, and has a “0” as its MSB to indicate data direction.
  _spi.transfer(addr & 0b01111111);
  delayMicroseconds(160); // tSRAD (160us)
  // read data
  uint8_t data = _spi.transfer(0);

  delayMicroseconds(1); // tSCLK-NCS(Read): 120ns

  digitalWrite(_ncsPin, HIGH);
  _spi.endTransaction();

  delayMicroseconds(19); // (tSRW/tSRR: 20us) - (tSCLK-NCS: 35us) = 19us

  return data;
}

void PMW3360::readMotionBurst(MotionBurstData &data, uint8_t length)
{
  length = min(static_cast<uint8_t>(12), length);

  // 1.Write any value to Motion_Burst register.
  writeRegister(Motion_Burst, 0);

  // 2.Lower NCS
  _spi.beginTransaction(spiSettings);
  digitalWrite(_ncsPin, LOW);

  // 3.Send Motion_Burst address (0x50).
  _spi.transfer(0x50);

  // 4.Wait for tSRAD_MOTBR: 35us
  delayMicroseconds(35);

  // 5.Start reading SPI Data continuously up to 12 bytes. Motion burst may be terminated by pulling NCS high for at least tBEXIT.
  _spi.transfer(data.raw, length);

  digitalWrite(_ncsPin, HIGH);
  _spi.endTransaction();

  delayMicroseconds(1); // tBEXIT: 500ns

  // 6.To read new motion burst data, repeat from step 2.
  // 7.If a non‐burst register read operation was executed; then, to read new burst data, start from step 1 instead.
}

void PMW3360::SROM_Download()
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
  _spi.beginTransaction(spiSettings);
  digitalWrite(_ncsPin, LOW);

  _spi.transfer(SROM_Load_Burst | 0b10000000);
  delayMicroseconds(15);

  // send all bytes of the firmware
  for (size_t i = 0; i < sizeof(PMW3360_Firmware); i++)
  {
    _spi.transfer(PMW3360_Firmware[i]);
    delayMicroseconds(15);
  }

  _spi.endTransaction();
  digitalWrite(_ncsPin, HIGH); // exit burst mode
  delayMicroseconds(185);      // 200us-15us

  // 7.Read the SROM_ID register to verify the ID before any other register reads or writes.
  readRegister(SROM_ID);

  // 8.Write 0x00 to Config2 register for wired mouse or 0x20 for wireless mouse design.
  writeRegister(Config2, 0x20);
}

void PMW3360::powerUp()
{
  // 1.Apply power to VDD and VDDIO in any order, with a delay of no more than 100ms in between each supply. Ensure all supplies are stable.

  // 2.Drive NCS high, and then low to reset the SPI port.
  digitalWrite(_ncsPin, HIGH);
  digitalWrite(_ncsPin, LOW);
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

void PMW3360::initRegisters()
{
  writeRegister(Control, PMW3360_Control);
  writeRegister(Config1, PMW3360_Config1);
  writeRegister(Angle_Tune, PMW3360_Angle_Tune);

  writeRegister(Run_Downshift, PMW3360_Run_Downshift);
  writeRegister(Rest1_Rate_Lower, PMW3360_Rest1_Rate & 0b0000000011111111);
  writeRegister(Rest1_Rate_Upper, (PMW3360_Rest1_Rate & 0b1111111100000000) >> 8);

  writeRegister(Rest1_Downshift, PMW3360_Rest1_Downshift);
  writeRegister(Rest2_Rate_Lower, PMW3360_Rest2_Rate & 0b0000000011111111);
  writeRegister(Rest2_Rate_Upper, (PMW3360_Rest2_Rate & 0b1111111100000000) >> 8);

  writeRegister(Rest2_Downshift, PMW3360_Rest2_Downshift);
  writeRegister(Rest3_Rate_Lower, PMW3360_Rest3_Rate & 0b0000000011111111);
  writeRegister(Rest3_Rate_Upper, (PMW3360_Rest3_Rate & 0b1111111100000000) >> 8);

  writeRegister(Lift_Config, PMW3360_Lift_Config);
}

void PMW3360::changeMode(Mode mode)
{
  if (_taskHandles[_id] == nullptr)
  {
    return;
  }

  uint32_t ms;
  uint8_t data;

  if (mode == Mode::Run)
  {
    ms = PMW3360_RUN_MODE_CALLBACK_INTERVAL;
    data = 0b00000000;
  }
  else
  {
    ms = PMW3360_REST_MODE_CALLBACK_INTERVAL;
    data = 0b00100000;
  }

  writeRegister(Config2, data);
  xTimerChangePeriod(_timerHandle, pdMS_TO_TICKS(ms), portMAX_DELAY);
}

void PMW3360::changeCpi(Cpi cpi)
{
  if (_taskHandles[_id] == nullptr)
  {
    return;
  }

  writeRegister(Config1, static_cast<uint8_t>(cpi));
}

void PMW3360::resetCpi()
{
  if (_taskHandles[_id] == nullptr)
  {
    return;
  }

  writeRegister(Config1, PMW3360_Config1);
}

void PMW3360::enableAngleSnap()
{
  if (_taskHandles[_id] == nullptr)
  {
    return;
  }

  writeRegister(Angle_Snap, 0b10000000);
}

void PMW3360::disableAngleSnap()
{
  if (_taskHandles[_id] == nullptr)
  {
    return;
  }

  writeRegister(Angle_Snap, 0b00000000);
}

#ifdef ARDUINO_ARCH_NRF52
void PMW3360::stopTask_and_setWakeUpInterrupt()
{
  vTaskSuspend(_taskHandles[_id]);

  NRF_GPIO->PIN_CNF[_interruptPin] |= ((uint32_t)(GPIO_PIN_CNF_SENSE_Low) << GPIO_PIN_CNF_SENSE_Pos);
}
#endif

} // namespace hidpg
