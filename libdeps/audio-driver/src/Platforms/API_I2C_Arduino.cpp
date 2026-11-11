// I2C Driver for Arduino
#if defined(ARDUINO) && !AUDIO_DRIVER_FORCE_IDF

#include <Wire.h>
#include <assert.h>
#include <stdio.h>

#include "Platforms/API_I2C.h"

error_t i2c_bus_create(struct I2CConfig *config) {
  AD_LOGI("i2c_bus_create");
  assert(config != nullptr);
  I2CConfig &pins = *config;
  TwoWire *p_wire = (TwoWire *)pins.p_wire;
  assert(p_wire != nullptr);

  // if no pins are defined, just call begin
  if (pins.scl == -1 || pins.sda == -1) {
    AD_LOGI("setting up I2C w/o pins");
    p_wire->begin();
  } else {
    // begin with defined pins, if supported
#if defined(ESP32)
    AD_LOGI("setting up I2C scl: %d, sda: %d", pins.scl, pins.sda);
    p_wire->begin(pins.sda,pins.scl);
#elif defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_STM32)
    AD_LOGI("setting up I2C scl: %d, sda: %d", pins.scl, pins.sda);
    p_wire->setSCL(pins.scl);
    p_wire->setSDA(pins.sda);
    p_wire->begin();
#else
    AD_LOGW("setting up I2C w/o pins");
    p_wire->begin();
#endif
    if (pins.frequency > 0) {
      AD_LOGI("Setting i2c clock: %u", pins.frequency);
      p_wire->setClock(pins.frequency);
    }
  }
  return RESULT_OK;
}

void i2c_bus_delete(i2c_bus_handle_t bus) {
  TwoWire *p_wire = (TwoWire *)bus;
  assert(p_wire != nullptr);
#if !defined(ESP8266) && FORCE_WIRE_CLOSE
  p_wire->end();
#endif
}

// this method is used !
error_t i2c_bus_write_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                            int reglen, uint8_t *data, int datalen) {
  AD_LOGD("i2c_bus_write_bytes: addr=0x%X reglen=%d datalen=%d - reg=0x%0X ",
          addr, reglen, datalen, reg[0]);
  TwoWire *p_wire = (TwoWire *)bus;
  assert(p_wire != nullptr);
  int result = RESULT_OK;
  p_wire->beginTransmission(addr);
  if (reglen > 0) p_wire->write(reg, reglen);
  if (datalen > 0) p_wire->write(data, datalen);
  int rc = p_wire->endTransmission(I2C_END);
  if (rc != 0) {
    AD_LOGE("->p_wire->endTransmission: %d", rc);
    result = RESULT_FAIL;
  }
  return result;
}

// this method is used !
error_t i2c_bus_read_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                           int reglen, uint8_t *outdata, int datalen) {
  AD_LOGD("i2c_bus_read_bytes: addr=0x%X reglen=%d datalen=%d - reg=0x%X", addr,
          reglen, datalen, reg[0]);
  TwoWire *p_wire = (TwoWire *)bus;
  assert(reglen == 1);
  assert(p_wire != nullptr);

  memset(outdata, 0, datalen);
  int result = RESULT_OK;

  p_wire->beginTransmission(addr);
  p_wire->write(reg, reglen);
  int rc = p_wire->endTransmission();
  if (rc != 0) {
    AD_LOGE("->p_wire->endTransmission: %d", rc);
  }

  uint8_t result_len = p_wire->requestFrom(addr, datalen, (int)I2C_END);
  if (result_len > 0) {
    result_len = p_wire->readBytes(outdata, datalen);
  } else {
    AD_LOGE("->p_wire->requestFrom %d->%d", datalen, result_len);
    result = RESULT_FAIL;
  }
  return result;
}

#endif