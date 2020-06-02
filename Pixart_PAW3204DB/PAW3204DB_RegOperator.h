#pragma once

#include <stdint.h>

class PAW3204DB_RegOperator
{
public:
  PAW3204DB_RegOperator(uint8_t sclk_pin, uint8_t sdio_pin);
  bool init();
  void write(uint8_t addr, uint8_t data);
  uint8_t read(uint8_t addr);
  void reSyncSerial();

private:
  const uint8_t _sclk_pin;
  const uint8_t _sdio_pin;
};

// #ifdef NRF52_SERIES
//   #define USE_REG_OPRATOR_NRF52
// #else
  #define USE_REG_OPRATOR_GPIO
// #endif
