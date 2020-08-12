#pragma once

#include <stdint.h>

namespace hidpg
{

  class PAW3204DB_RegOperator
  {
  public:
    virtual bool begin() = 0;
    virtual void write(uint8_t addr, uint8_t data) = 0;
    virtual uint8_t read(uint8_t addr) = 0;
    virtual void reSyncSerial() = 0;
  };

} // namespace hidpg
