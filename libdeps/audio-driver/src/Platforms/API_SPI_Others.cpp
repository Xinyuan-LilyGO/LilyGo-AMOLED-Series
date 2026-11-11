#ifndef ARDUINO
#include "API_SPI.h"

error_t spi_bus_create(struct SPIConfig *config) {
  AD_LOGI("spi_bus (does nothing)");
  return RESULT_OK;
}

void spi_bus_delete(spi_bus_handle_t bus) {}

#endif