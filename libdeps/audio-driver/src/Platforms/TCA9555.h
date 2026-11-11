#pragma once
/**
 * @file tca9555.h
 * @brief Header-only driver for the TCA9555 I2C I/O expander.
 * @author (c) 2025
 */

#include "API_GPIO.h"
#include "Platforms/API_I2C.h"
#include "Platforms/IDriverPins.h"

/**
 * @namespace audio_driver
 * @brief Namespace for audio driver components.
 */
namespace audio_driver {

/**
 * @class TCA9555
 * @brief Driver for the TCA9555 I2C I/O expander (header-only).
 *
 * Provides methods to configure pin direction, read and write pin states, and
 * manage the I2C bus.
 */
class TCA9555 : public API_GPIO {
 public:
  /**
   * @brief Construct a new TCA9555 object with default I2C address 0x20.
   */
  TCA9555() : i2c_default_address(0x20), bus(nullptr) {}

  /**
   * @brief Initialize the TCA9555 and create the I2C bus.
   * @param pins DriverPins structure with SCL and SDA pin assignments.
   * @return true if initialization was successful, false otherwise.
   */
  bool begin(IDriverPins& pins) override {
    AD_LOGI("TCA9555::begin");
    auto expanderPins = pins.getI2CPins(PinFunction::EXPANDER);
    if (!expanderPins) {
      AD_LOGI("TCA9555 not configured in DriverPins");
      return false;
    }
    // Setup I2CConfig and create the bus
    I2CConfig cfg = expanderPins.value();
    if (cfg.address > 0) {
      i2c_default_address = cfg.address;
    }
    AD_LOGI("TCA9555 address: 0x%02X", i2c_default_address);
    i2c_bus_create(&cfg);
    bus = cfg.p_wire;
    return bus != nullptr;
  }

  /**
   * @brief Deinitialize the TCA9555 and delete the I2C bus.
   * @return true always.
   */
  void end() override {
    if (bus != nullptr) {
      i2c_bus_delete(bus);
      bus = nullptr;
    }
  }

  /**
   * @brief Set the output state of a pin.
   * @param pin Pin number (0-15).
   * @param value true for HIGH, false for LOW.
   * @return true if successful, false otherwise.
   */
  bool digitalWrite(int pin, bool value) override {
    if (pin > 1000) pin -= 1000;
    AD_LOGI("TCA9555::digitalWrite %d: %d", pin, value);
    if (pin > 15 || bus == nullptr) {
      AD_LOGE("TCA9555 invalid pin: %d", pin);
      return false;
    }
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = 0x02 + port;  // Output port 0 or 1
    uint8_t out;
    if (!i2c_read(i2c_default_address, reg, &out, 1)) {
      AD_LOGE("TCA9555 read failed for pin: %d", pin);
      return false;
    }
    if (value)
      out |= (1 << bit);
    else
      out &= ~(1 << bit);
    bool rc = i2c_write(i2c_default_address, reg, &out, 1);
    if (!rc) {
      AD_LOGE("TCA9555 write failed for pin: %d", pin);
    }
    return rc;
  }

  /**
   * @brief Read the input state of a pin.
   * @param pin Pin number (0-15).
   * @return true if HIGH, false if LOW or on error.
   */
  bool digitalRead(int pin) override {
    if (pin > 1000) pin -= 1000;
    AD_LOGI("TCA9555::digitalRead %d", pin);
    if (pin > 15 || bus == nullptr) return false;
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = 0x00 + port;  // Input port 0 or 1
    uint8_t in;
    if (!i2c_read(i2c_default_address, reg, &in, 1)) return false;
    return (in & (1 << bit)) != 0;
  }

  /**
   * @brief Set the direction of a pin.
   * @param pin Pin number (0-15).
   * @param input true for input, false for output.
   * @return true if successful, false otherwise.
   */
  void pinMode(int pin, int mode) override {
    if (pin > 1000) pin -= 1000;
    AD_LOGI("TCA9555::pinMode: %d", pin);
    if (pin > 15 || bus == nullptr) {
      AD_LOGE("TCA9555 pinMode: invalid pin: %d", pin);
      return;
    }
    bool input = (mode == INPUT || mode == INPUT_PULLUP);
    uint8_t port = pin / 8;
    uint8_t bit = pin % 8;
    uint8_t reg = 0x06 + port;  // Configuration port 0 or 1
    uint8_t cfg;
    if (!i2c_read(i2c_default_address, reg, &cfg, 1)) {
      AD_LOGE("TCA9555 read failed for pin: %d", pin);
      return;
    }
    if (input)
      cfg |= (1 << bit);
    else
      cfg &= ~(1 << bit);
    i2c_write(i2c_default_address, reg, &cfg, 1);
  }

  /**
   * @brief Set the I2C address for the TCA9555.
   * @param address The new I2C address.
   */
  void setI2CAddress(uint8_t address) { i2c_default_address = address; }

 protected:
  uint8_t i2c_default_address;
  i2c_bus_handle_t bus = nullptr;

  /**
   * @brief Helper to read bytes from the TCA9555 using the platform I2C
   * abstraction.
   */
  bool i2c_read(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) {
    if (bus == nullptr) {
      AD_LOGE("TCA9555 bus is null");
      return false;
    }
    return i2c_bus_read_bytes(bus, addr, &reg, 1, data, len) == 0;
  }
  /**
   * @brief Helper to write bytes to the TCA9555 using the platform I2C
   * abstraction.
   */
  bool i2c_write(uint8_t addr, uint8_t reg, uint8_t* data, size_t len) {
    if (bus == nullptr) {
      AD_LOGE("TCA9555 bus is null");
      return false;
    }
    return i2c_bus_write_bytes(bus, addr, &reg, 1, data, len) == 0;
  }
};

}  // namespace audio_driver