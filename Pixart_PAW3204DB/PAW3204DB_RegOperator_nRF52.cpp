
#include "Arduino.h"
#include "PAW3204DB_RegOperator.h"

#ifdef USE_REG_OPRATOR_NRF52

namespace hidpg
{

  PAW3204DB_RegOperator::PAW3204DB_RegOperator(uint8_t sclk_pin, uint8_t sdio_pin)
      : _sclk_pin(sclk_pin), _sdio_pin(sdio_pin)
  {
  }

  bool PAW3204DB_RegOperator::begin()
  {
    pinMode(_sclk_pin, OUTPUT);
    pinMode(_sdio_pin, INPUT);

    return true;
  }

  void PAW3204DB_RegOperator::write(uint8_t addr, uint8_t data)
  {
  }

  uint8_t PAW3204DB_RegOperator::read(uint8_t addr)
  {
  }

  void PAW3204DB_RegOperator::reSyncSerial()
  {
    // Re-Synchronous Serial Interface
    digitalWrite(_sclk_pin, HIGH);
    delay(1);
    digitalWrite(_sclk_pin, LOW);
    delayMicroseconds(1); // tRESYNC
    digitalWrite(_sclk_pin, HIGH);
    delay(512); //tSIWTT(max 512ms)
  }

} // namespace hidpg

#endif
