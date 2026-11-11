#pragma once
/**
 * @file ESP32S3AISmartSpeaker.h
 * @brief Device definition for ESP32 S3 AI Smart Speaker with TCA9555 GPIO exp
 */

#include "AudioBoard.h"
#include "FS.h"
#include "SD_MMC.h"

#define EXIO1 1000
#define EXIO2 1001
#define EXIO3 1002
#define EXIO4 1003
#define EXIO5 1004
#define EXIO6 1005
#define EXIO7 1006
#define EXIO8 1007
#define EXIO9 1008
#define EXIO10 1009
#define EXIO11 1010
#define EXIO12 1011
#define EXIO13 1012
#define EXIO14 1013
#define EXIO15 1014
#define EXIO16 1015

namespace audio_driver {

/**
 * @brief Pins for ESP32 S3 AI Smart Speaker - use the PinsESP32S3AISmartSpeaker
 * object! We support the TCA9555 GPIO expander!
 * @author Phil Schatzmann
 */
class PinsESP32S3AISmartSpeakerClass : public DriverPins {
 public:
  PinsESP32S3AISmartSpeakerClass() {
    // setup TCA9555 GPIO expander
    gpio.setAltGPIO(tca9555, 1000);
    // CLK, MISO (DATA), MOSI (CMD), CS
    PinsSPI sd{PinFunction::SD, 40, 41, 42, -1, SPI};

    addSPI(sd);
    // add i2c codec pins: scl, sda, port, frequency
    addI2C(PinFunction::EXPANDER, 10, 11);
    // addI2C(PinFunction::CODEC, 10, 11);
    //  add i2s pins: mclk, bck, ws,data_out, data_in ,(port)
    addI2S(PinFunction::CODEC, 12, 13, 14, 16, 15);

    // add other pins
    addPin(PinFunction::KEY, EXIO11, PinLogic::InputActiveLow, 1);
    addPin(PinFunction::KEY, EXIO12, PinLogic::InputActiveLow, 2);
    addPin(PinFunction::KEY, EXIO13, PinLogic::InputActiveLow, 3);
    addPin(PinFunction::PA, EXIO9, PinLogic::Output);
    addPin(PinFunction::SD, EXIO4, PinLogic::Output);  // SD CS
    addPin(PinFunction::LED, 38, PinLogic::Output);
  }

  bool begin() override {
    AD_LOGD("PinsESP32S3AISmartSpeakerClass::begin");
    bool rc = DriverPins::begin();
    // activate SD CS
    if (sd_active) {
      AD_LOGD("Activate SD CS");
      tca9555.digitalWrite(EXIO4, true);  // activate SD CS
    }
    // activate SDMMC
    if (sdmmc_active) {
      AD_LOGD("Activate SDMMC");
      if (!SD_MMC.setPins(40, 42, 41, -1, -1, -1)) {
        AD_LOGE("SDMMC setPins failed");
      }
      tca9555.digitalWrite(EXIO4, true);  // deactivate SD CS
    }
    return rc;
  }

  TCA9555& getTCA9555() { return tca9555; }

 protected:
  TCA9555 tca9555;
};

/// @ingroup audio_driver
static PinsESP32S3AISmartSpeakerClass PinsESP32S3AISmartSpeaker;

/// @ingroup audio_driver
static AudioBoard ESP32S3AISmartSpeaker{AudioDriverES8311_ES7210,
                                        PinsESP32S3AISmartSpeaker};

}  // namespace audio_driver