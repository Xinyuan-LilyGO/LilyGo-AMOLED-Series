#ifdef ARDUINO
#include "API_SPI.h"
#include "SPI.h"

error_t spi_bus_create(struct SPIConfig *config) {
  SPIConfig &pins = *config;
  SPIClass *p_spi = (SPIClass *)pins.p_spi;
  assert(p_spi != nullptr);
  p_spi->end();
  // setup chip select
  if (pins.cs != -1) {
    pinMode(pins.cs, OUTPUT);
    digitalWrite(pins.cs, HIGH);
  }
  // if no pins are defined, just call begin
  if (pins.miso == -1 || pins.mosi == -1 || pins.clk == -1) {
    AD_LOGI("setting up SPI w/o pins");
    p_spi->begin();
  } else {
// begin spi and set up pins if supported
#if defined(ARDUINO_ARCH_STM32)
    AD_LOGI("setting up SPI miso:%d,mosi:%d, clk:%d, cs:%d", pins.miso,
            pins.mosi, pins.clk, pins.cs);
    p_spi->setMISO(pins.miso);
    p_spi->setMOSI(pins.mosi);
    p_spi->setSCLK(pins.clk);
    p_spi->setSSEL(pins.cs);
    p_spi->begin();
#elif defined(ESP32)
    AD_LOGI("setting up SPI miso:%d,mosi:%d, clk:%d, cs:%d", pins.miso,
            pins.mosi, pins.clk, pins.cs);
    p_spi->begin(pins.clk, pins.miso, pins.mosi, pins.cs);
#elif defined(ARDUINO_ARCH_AVR)
    AD_LOGW("setting up SPI w/o pins");
    p_spi->begin();
#endif
  }
  return RESULT_OK;
}

void spi_bus_delete(spi_bus_handle_t bus) {
  AD_LOGD("PinsSPI::end");
  SPIClass *p_spi = (SPIClass *)bus;
  p_spi->end();
}

#endif