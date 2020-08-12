#pragma once

#ifdef ARDUINO_ARCH_NRF52

#include "PAW3204DB_RegOperator.h"
#include "nrf_spi.h"

namespace hidpg
{

  class PAW3204DB_RegOperator_nRF52 : public PAW3204DB_RegOperator
  {
  public:
    PAW3204DB_RegOperator_nRF52(NRF_SPI_Type *spi, uint8_t sclk_pin, uint8_t sdio_pin);
    bool begin() override;
    void write(uint8_t addr, uint8_t data) override;
    uint8_t read(uint8_t addr) override;
    void reSyncSerial() override;

  private:
    NRF_SPI_Type *_spi;
    const uint8_t _sclk_pin;
    const uint8_t _sdio_pin;
  };

} // namespace hidpg

#endif
