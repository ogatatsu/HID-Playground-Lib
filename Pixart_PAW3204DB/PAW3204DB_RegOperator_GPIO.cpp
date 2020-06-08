
#include "Arduino.h"
#include "PAW3204DB_RegOperator.h"

#ifdef USE_REG_OPRATOR_GPIO

namespace hidpg
{

  PAW3204DB_RegOperator::PAW3204DB_RegOperator(uint8_t sclk_pin, uint8_t sdio_pin)
      : _sclk_pin(sclk_pin), _sdio_pin(sdio_pin)
  {
  }

  bool PAW3204DB_RegOperator::init()
  {
    pinMode(_sclk_pin, OUTPUT);
    pinMode(_sdio_pin, INPUT);

    return true;
  }

  void PAW3204DB_RegOperator::write(uint8_t addr, uint8_t data)
  {
    uint16_t data16 = ((addr | 0b10000000) << 8) | data;

    pinMode(_sdio_pin, OUTPUT);
    for (int i = 15; i >= 0; i--)
    {
      digitalWrite(_sclk_pin, LOW);
      digitalWrite(_sdio_pin, bitRead(data16, i));
      digitalWrite(_sclk_pin, HIGH);
    }
    pinMode(_sdio_pin, INPUT);
  }

  uint8_t PAW3204DB_RegOperator::read(uint8_t addr)
  {
    // addr &= 0b01111111;
    uint8_t data = 0;

    pinMode(_sdio_pin, OUTPUT);
    for (int i = 7; i >= 0; i--)
    {
      digitalWrite(_sclk_pin, LOW);
      digitalWrite(_sdio_pin, bitRead(addr, i));
      digitalWrite(_sclk_pin, HIGH);
    }

    pinMode(_sdio_pin, INPUT);
    delayMicroseconds(3); //tHOLD

    for (int i = 7; i >= 0; i--)
    {
      digitalWrite(_sclk_pin, LOW);
      digitalWrite(_sclk_pin, HIGH);
      uint8_t bitvalue = digitalRead(_sdio_pin);
      data |= bitvalue << i;
    }

    return data;
  }

  void PAW3204DB_RegOperator::reSyncSerial()
  {
    // Re-Synchronous Serial Interface
    digitalWrite(_sclk_pin, HIGH);
    delay(1);
    digitalWrite(_sclk_pin, LOW);
    delayMicroseconds(1); // tRESYNC
    digitalWrite(_sclk_pin, HIGH);
    delay(320); //tSIWTT(max 320ms)
  }

} // namespace hidpg

#endif
