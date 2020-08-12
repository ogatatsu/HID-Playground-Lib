
#ifdef ARDUINO_ARCH_NRF52

#include "PAW3204DB_RegOperator_nRF52.h"
#include "Arduino.h"

namespace hidpg
{

  PAW3204DB_RegOperator_nRF52::PAW3204DB_RegOperator_nRF52(NRF_SPI_Type *spi, uint8_t sclk_pin, uint8_t sdio_pin)
      : _spi(spi), _sclk_pin(sclk_pin), _sdio_pin(sdio_pin)
  {
  }

  bool PAW3204DB_RegOperator_nRF52::begin()
  {
    // set pin
    pinMode(_sclk_pin, OUTPUT);
    pinMode(_sdio_pin, INPUT);

    // init spi
    nrf_spi_configure(_spi, NRF_SPI_MODE_3, NRF_SPI_BIT_ORDER_MSB_FIRST);
    nrf_spi_frequency_set(_spi, NRF_SPI_FREQ_8M);
    nrf_spi_event_clear(_spi, NRF_SPI_EVENT_READY);

    return true;
  }

  void PAW3204DB_RegOperator_nRF52::write(uint8_t addr, uint8_t data)
  {
    // set sdio pin to mosi
    nrf_spi_pins_set(_spi, g_ADigitalPinMap[_sclk_pin], g_ADigitalPinMap[_sdio_pin], 0xFF);

    nrf_spi_enable(_spi);

    // write addres
    nrf_spi_txd_set(_spi, addr | 0b10000000);
    while (!nrf_spi_event_check(_spi, NRF_SPI_EVENT_READY))
      ;
    (void)nrf_spi_rxd_get(_spi);
    nrf_spi_event_clear(_spi, NRF_SPI_EVENT_READY);

    // write data
    nrf_spi_txd_set(_spi, data);
    while (!nrf_spi_event_check(_spi, NRF_SPI_EVENT_READY))
      ;
    (void)nrf_spi_rxd_get(_spi);
    nrf_spi_event_clear(_spi, NRF_SPI_EVENT_READY);

    nrf_spi_disable(_spi);
  }

  uint8_t PAW3204DB_RegOperator_nRF52::read(uint8_t addr)
  {
    // set sdio pin to mosi
    nrf_spi_pins_set(_spi, g_ADigitalPinMap[_sclk_pin], g_ADigitalPinMap[_sdio_pin], 0xFF);

    // write addres
    nrf_spi_enable(_spi);
    nrf_spi_txd_set(_spi, addr);
    while (!nrf_spi_event_check(_spi, NRF_SPI_EVENT_READY))
      ;
    (void)nrf_spi_rxd_get(_spi);
    nrf_spi_event_clear(_spi, NRF_SPI_EVENT_READY);
    nrf_spi_disable(_spi);

    // set sdio pin to miso
    nrf_spi_pins_set(_spi, g_ADigitalPinMap[_sclk_pin], 0xFF, g_ADigitalPinMap[_sdio_pin]);
    delayMicroseconds(3); //tHOLD

    // read data
    nrf_spi_enable(_spi);
    nrf_spi_txd_set(_spi, 0);
    while (!nrf_spi_event_check(_spi, NRF_SPI_EVENT_READY))
      ;
    uint8_t data = nrf_spi_rxd_get(_spi);
    nrf_spi_event_clear(_spi, NRF_SPI_EVENT_READY);
    nrf_spi_disable(_spi);

    return data;
  }

  void PAW3204DB_RegOperator_nRF52::reSyncSerial()
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
