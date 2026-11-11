#pragma once

namespace audio_driver_local {

/**
 * @brief A simple optonal implementation
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
template <typename T> class Optional {
public:
  Optional() {}
  Optional(T value) {
    val = value;
    is_valid = true;
  }
  operator bool() const { return is_valid; }
  T &value() { return val; }
  T &valueOr(T alt) { return is_valid ? val : alt; }
  void reset() { is_valid = false; }

protected:
  bool is_valid = false;
  T val;
};

}
