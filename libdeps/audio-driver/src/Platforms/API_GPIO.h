#pragma once
#include "IDriverPins.h"

namespace audio_driver {

class API_GPIO {
 public:
  virtual bool begin(IDriverPins& pins) = 0;
  virtual void end() = 0;
  virtual void pinMode(int pin, int mode) = 0;
  virtual bool digitalWrite(int pin, bool value) = 0;
  virtual bool digitalRead(int pin) = 0;
};

}