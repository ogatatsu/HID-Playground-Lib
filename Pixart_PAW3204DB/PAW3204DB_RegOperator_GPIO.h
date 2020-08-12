#pragma once

#include "PAW3204DB_RegOperator.h"

namespace hidpg
{

  class PAW3204DB_RegOperator_GPIO : public PAW3204DB_RegOperator
  {
  public:
    PAW3204DB_RegOperator_GPIO(uint8_t sclk_pin, uint8_t sdio_pin);
    bool begin() override;
    void write(uint8_t addr, uint8_t data) override;
    uint8_t read(uint8_t addr) override;
    void reSyncSerial() override;

  private:
    const uint8_t _sclk_pin;
    const uint8_t _sdio_pin;
  };

} // namespace hidpg
