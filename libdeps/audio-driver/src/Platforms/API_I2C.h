#pragma once
#include <stdint.h>
#include "DriverCommon.h"

typedef int16_t GpioPin;

struct I2CConfig {
  uint32_t frequency;
  int port;
  GpioPin scl;
  GpioPin sda;
  i2c_bus_handle_t p_wire;
  int address;
};

#ifndef ARDUINO
struct TwoWire {};
#endif

#ifdef __cplusplus
extern "C" {
#endif

error_t i2c_bus_create(struct I2CConfig *pin);

void i2c_bus_delete(i2c_bus_handle_t bus);

/**
 * @brief Write bytes to I2C bus
 *
 * @param bus        I2C bus handle
 * @param addr       The address of the device
 * @param reg        The register of the device
 * @param regLen     The length of register
 * @param data       The data pointer
 * @param datalen    The length of data
 *
 * @return
 *     - NULL Fail
 *     - Others Success
 */
error_t i2c_bus_write_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                            int regLen, uint8_t *data, int datalen);

/**
 * @brief Read bytes to I2C bus
 *
 * @param bus        I2C bus handle
 * @param addr       The address of the device
 * @param reg        The register of the device
 * @param regLen     The length of register
 * @param outdata    The outdata pointer
 * @param datalen        The length of outdata
 *
 * @return
 *     - NULL Fail
 *     - Others Success
 */
error_t i2c_bus_read_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                           int reglen, uint8_t *outdata, int datalen);

#ifdef __cplusplus
}
#endif
