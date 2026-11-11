#pragma once
#include <stdint.h>

#include "Platforms/GPIO.h"
#include "Platforms/TCA9555.h"

/**
 * @file GPIO.h
 * @brief Simple GPIO API for digital input/output providing the
 * the standard Arduino GPIO and an optional extender (e.g. TCA9555).
 */

namespace audio_driver {

/**
 * @class GPIO
 * @brief Abstraction for digital GPIO pin operations.
 */
class GPIOExt : public API_GPIO {
 public:
  /**
   * @brief Construct a new GPIO object.
   * @param pin The pin number or handle.
   */
  GPIOExt() = default;
  bool begin(IDriverPins& pins) override {
    bool rc = gpio.begin(pins);
    if (p_ext) p_ext->begin(pins);
    return rc;
  }
  void end() override {
    gpio.end();
    if (p_ext) p_ext->end();
  }
  void pinMode(int pin, int mode) override {
    if (p_ext && pin >= offset) {
      p_ext->pinMode(pin, mode);
    } else {
      gpio.pinMode(pin, mode);
    }
  }
  bool digitalWrite(int pin, bool value) override {
    if (p_ext && pin >= offset) {
      return p_ext->digitalWrite(pin, value);
    } else {
      return gpio.digitalWrite(pin, value);
    }
  }

  bool digitalRead(int pin) override {
    if (p_ext && pin >= offset) {
      return p_ext->digitalRead(pin);
    } else {
      return gpio.digitalRead(pin);
    }
  }

  void setAltGPIO(API_GPIO& gpioExt, int offset = 1000) {
    AD_LOGD("setAltGPIO");
    p_ext = &gpioExt;
    this->offset = offset;
  }

 protected:
  int offset = 1000;
  GPIO gpio; // default GPIO
  API_GPIO* p_ext = nullptr; // alternative GPIO
};

}  // namespace audio_driver