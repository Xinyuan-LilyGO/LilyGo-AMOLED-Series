#pragma once
#include <stdint.h>

#include "API_GPIO.h"
#include "Arduino.h"

/**
 * @file GPIO.h
 * @brief Simple GPIO abstraction for digital input/output.
 */

namespace audio_driver {

/**
 * @class GPIO
 * @brief Abstraction for digital GPIO pin operations.
 */
class GPIO : public API_GPIO {
 public:
  /**
   * @brief Construct a new GPIO object.
   * @param pin The pin number or handle.
   */
  GPIO() = default;
  bool begin(IDriverPins& pins) { return true; }
  void end() {}
  void pinMode(int pin, int mode) { ::pinMode(pin, mode); }
  bool digitalWrite(int pin, bool value) {
    ::digitalWrite(pin, value);
    return true;
  }
  bool digitalRead(int pin) { return ::digitalRead(pin); }
};

}  // namespace audio_driver