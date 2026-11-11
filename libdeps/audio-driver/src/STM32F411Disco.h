#pragma once
/**
 * @file STM32F411Disco.h
 * @brief Device definition for STM32F411 Discovery board
 */

#include "AudioBoard.h"

namespace audio_driver {

/**
 * @brief Pins the STM32F411Disco board using the CS43L22 codec
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class PinsSTM32F411DiscoClass : public DriverPins {
 public:
  PinsSTM32F411DiscoClass() {
    // add i2c codec pins: scl, sda, port, frequency
    addI2C(PinFunction::CODEC, PB6, PB9);
    // add i2s pins: mclk, bck, ws,data_out, data_in ,(port)
    addI2S(PinFunction::CODEC, PC7, PC10, PA4, PC3, PC12);

    // add other pins
    addPin(PinFunction::KEY, PA0, PinLogic::Output);      // user button
    addPin(PinFunction::LED, PD12, PinLogic::Output, 0);  // green
    addPin(PinFunction::LED, PD5, PinLogic::Output, 1);   // red
    addPin(PinFunction::LED, PD13, PinLogic::Output, 2);  // orange
    addPin(PinFunction::LED, PD14, PinLogic::Output, 3);  // red
    addPin(PinFunction::LED, PD15, PinLogic::Output, 4);  // blue
    addPin(PinFunction::PA, PD4, PinLogic::Output);  // reset pin (active high)
    // addPin(PinFunction::CODEC_ADC, PC3, PinLogic::Input); // Microphone
  }
};

/// @ingroup audio_driver
static PinsSTM32F411DiscoClass PinsSTM32F411Disco;

/// @ingroup audio_driver
static AudioBoard STM32F411Disco{AudioDriverCS43l22, PinsSTM32F411Disco};

}