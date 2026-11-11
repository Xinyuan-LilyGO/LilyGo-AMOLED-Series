
#pragma once
#include "DriverCommon.h"
#include "Platforms/API_I2C.h"
#include "Platforms/API_SPI.h"
#include "Platforms/API_GPIO.h"
#include "Platforms/Logger.h"
#include "Platforms/Optional.h"
#include "Platforms/Vector.h"
//#include "Platforms/TCA9555.h"
#include "Platforms/IDriverPins.h"

#ifdef ARDUINO
#include "Wire.h"
#define DEFAULT_WIRE &Wire
#else
#define DEFAULT_WIRE nullptr
#endif

#ifndef TOUCH_LIMIT
#define TOUCH_LIMIT 20
#endif

#ifndef LYRAT_MINI_RANGE
#define LYRAT_MINI_RANGE 5
#endif

#ifndef LYRAT_MINI_DELAY_MS
#define LYRAT_MINI_DELAY_MS 5
#endif

namespace audio_driver {

/**
 * @enum PinLogic
 * @brief input or output
 * @ingroup enumerations
 */

enum class PinLogic {
  InputActiveHigh,
  InputActiveLow,
  InputActiveTouch,
  Input,
  Output,
  Inactive,
};

/**
 * @enum PinFunction
 * @brief Pin Functions
 * @ingroup enumerations
 * @ingroup audio_driver
 */
enum class PinFunction {
  UNDEFINED = 0,
  HEADPHONE_DETECT,
  AUXIN_DETECT,
  PA,  // Power Amplifier
  POWER,
  LED,
  KEY,
  SD,
  CODEC,
  CODEC_ADC,
  LATCH,
  RESET,
  MCLK_SOURCE,
  EXPANDER,
};

/**
 * @enum AudioDriverKey
 * @brief Key names
 * @ingroup enumerations
 * @ingroup audio_driver
 */
enum AudioDriverKey {
  KEY_REC = 0,
  KEY_MODE,
  KEY_PLAY,
  KEY_SET,
  KEY_VOLUME_DOWN,
  KEY_VOLUME_UP
};


/**
 * @brief I2S pins
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct PinsI2S {
  PinsI2S() = default;
  PinsI2S(PinFunction function, GpioPin mclk, GpioPin bck, GpioPin ws,
          GpioPin data_out, GpioPin data_in = -1, int port = 0) {
    this->function = function;
    this->mclk = mclk;
    this->bck = bck;
    this->ws = ws;
    this->data_out = data_out;
    this->data_in = data_in;
    this->port = port;
  }
  PinFunction function;
  GpioPin mclk;
  GpioPin bck;
  GpioPin ws;
  GpioPin data_out;
  GpioPin data_in;
  int port;  // port number
};

/**
 * @brief SPI pins: In Arduino we initialize the SPI, on other platform
 * we just provide the pin information
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct PinsSPI : public SPIConfig {
  PinsSPI() {
    this->clk = -1;
    this->miso = -1;
    this->mosi = -1;
    this->cs = -1;
    this->p_spi = &SPI;
  };
  PinsSPI(PinFunction function, GpioPin clk, GpioPin miso, GpioPin mosi,
          GpioPin cs, SPIClass& spi = SPI) {
    this->function = function;
    this->clk = clk;
    this->miso = miso;
    this->mosi = mosi;
    this->cs = cs;
    this->p_spi = &spi;
  }

  PinFunction function;
  bool set_active = true;
  bool pinsAvailable() { return clk != -1 && miso != -1 && mosi != -1; }
  operator bool() { return pinsAvailable(); }
  bool begin() {
    if (set_active) {
      AD_LOGD("PinsSPI::begin for %d", (int)function);
      spi_bus_create(this);
    } else {
      AD_LOGI("SPI, not active, MOSI, MISO, SCLK, SSEL not modified");
    }
    return true;
  }
  void end() {
    AD_LOGD("PinsSPI::end");
    spi_bus_delete(p_spi);
  }
};

/**
 * @brief Default SPI pins for ESP32 Lyrat, AudioDriver etc
 * CLK, MISO, MOSI, CS
 */

static PinsSPI ESP32PinsSD{PinFunction::SD, 14, 2, 15, 13, SPI};

/**
 * @brief I2C pins
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct PinsI2C : public I2CConfig {
  PinsI2C() {
    port = 0;
    address = -1;
    scl = -1;
    sda = -1;
    frequency = 100000;
    set_active = true;
    p_wire = DEFAULT_WIRE;
  };

  PinsI2C(PinFunction function, GpioPin scl, GpioPin sda, int address = -1,
          uint32_t frequency = 100000, i2c_bus_handle_t wire = DEFAULT_WIRE,
          bool active = true) {
    this->function = function;
    this->scl = scl;
    this->sda = sda;
    this->port = 0;
    this->frequency = frequency;
    this->p_wire = wire;
    this->set_active = active;
    this->address = address;
  }

  PinFunction function;
  bool set_active = true;
  bool pinsAvailable() { return scl != -1 && sda != -1 && frequency != 0; }
  operator bool() { return pinsAvailable(); }

  bool begin() {
    if (set_active) {
      AD_LOGD("PinsI2C::begin for function %d on port %d", (int)function, port);
      return i2c_bus_create(this) == RESULT_OK;
    }
    return true;
  }
  void end() {
    if (set_active) i2c_bus_delete(p_wire);
  }
};

/**
 * @brief Pins for LED, Buttons, AMP etc
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct PinsFunction {
  PinsFunction() = default;
  PinsFunction(PinFunction function, GpioPin pin, PinLogic logic,
               int index = 0) {
    this->function = function;
    this->pin = pin;
    this->index = index;
    this->pin_logic = logic;
  }
  PinFunction function;
  int pin = -1;
  int index = 0;
  PinLogic pin_logic;
  bool active = true;  // false if pin conflict
};

class IDriverPins {
 public:
   virtual audio_driver_local::Optional<PinsI2C> getI2CPins(PinFunction function) = 0;
};

}