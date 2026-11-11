/* Adapted from Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 * Copyright (c) 2024, phil schatzmann
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <math.h>

#include "Platforms/API_I2C.h"

/***
 * Set up the PCM3168 in slave mode
 */

class PCM3168 {
  enum PCM3168reg {
    RESET_CONTROL = 0x40,
    DAC_CONTROL_1,
    DAC_CONTROL_2,
    DAC_OUTPUT_PHASE,
    DAC_SOFT_MUTE_CONTROL,
    DAC_ZERO_FLAG,
    DAC_CONTROL_3,
    DAC_ATTENUATION_ALL,
    DAC_ATTENUATION_BASE /* 8 registers */,
    ADC_SAMPLING_MODE = 0x50,
    ADC_CONTROL_1,
    ADC_CONTROL_2,
    ADC_INPUT_CONFIGURATION,
    ADC_INPUT_PHASE,
    ADC_SOFT_MUTE_CONTROL,
    ADC_OVERFLOW_FLAG,
    ADC_CONTROL_3,
    ADC_ATTENUATION_ALL,
    ADC_ATTENUATION_BASE /* 6 registers */
  };

 public:
  enum FMT {
    I2S24bit = 0,
    Left24bit,
    Right24bit,
    Right16bit,
    I2SDSP24bit,
    LeftDSP24bit,
    I2STDM24bit,
    LeftTDM24bit,
    I2SHighSpeedTDM24bit,
    LeftHighSpeedTDM24bit,
  };

  PCM3168() = default;

  void setAddress(uint8_t addr) { i2c_addr = addr; }

  void setWire(i2c_bus_handle_t w) { wire = w; }

  bool begin(FMT fmt = I2SHighSpeedTDM24bit) {
    //wire->begin();
    return write(DAC_CONTROL_1, fmt) && write(ADC_CONTROL_1, fmt) &&
           setMute(false) && setMuteADC(false);
  }

  bool end(void) {
    // not really disabled, but it is muted
    return setMute(true) && setMuteADC(true);
  }

  bool setMute(int channel, bool mute) {
    mute_dac = bit_set_to(mute_dac, channel, mute);
    return write(DAC_SOFT_MUTE_CONTROL, mute_dac);
  }

  bool setMute(bool mute) {
    mute_dac = mute ? 0xFF : 0x00;
    return write(DAC_SOFT_MUTE_CONTROL, mute_dac);
  }

  bool setMuteADC(bool mute) {
    mute_adc = mute ? 0xFF : 0x00;
    return write(ADC_SOFT_MUTE_CONTROL, mute_dac);
  }

  bool setVolume(float level) { return volumeInteger(volumebyte(level)); }

  bool setVolume(int channel, float level) {
    if (channel < 1 || channel > DAC_CHANNELS_MAX) return false;
    return volumeInteger(channel, volumebyte(level));
  }

  bool setInputVolume(float level) {
    return inputLevelInteger(inputlevelbyte(level));
  }

  bool setInputVolume(int channel, float level) {
    if (channel < 1 || channel > DAC_CHANNELS_MAX) return false;
    return inputLevelInteger(channel, inputlevelbyte(level));
  }

  // these bits will invert the signal polarity of their respective DAC
  // channels (1-8)
  bool invertDAC(uint32_t data) { return write(DAC_OUTPUT_PHASE, data); }

  // these bits will invert the signal polarity of their respective ADC
  // channels (1-8)
  bool invertADC(uint32_t data) { return write(ADC_INPUT_PHASE, data); }

 protected:
  const uint8_t I2C_BASE = 0x44;
  const int ADC_CHANNELS_MAX = 6;
  const int DAC_CHANNELS_MAX = 8;
  i2c_bus_handle_t wire = nullptr;
  uint8_t i2c_addr = I2C_BASE;
  FMT fmt;
  uint8_t mute_dac = 0;
  uint8_t mute_adc = 0;

  inline uint8_t bit_set_to(uint8_t number, uint8_t n, bool x) {
    return (number & ~((uint8_t)1 << n)) | ((uint8_t)x << n);
  }

  // convert level to volume byte, Table 21, page 45
  uint32_t volumebyte(float level) {
    if (level >= 1.0f) return 255;
    if (level <= 1.0e-5f) return 54;  // below -100dB, mute
    // 0.5dB steps: 0dB attenuation is 255
    return 255 + roundf(log10f(level) * 40.0f);
  }

  // convert level to input gain, Table 20, page 50
  int32_t inputlevelbyte(float level) {
    if (level > 10.0) return 255;
    if (level < 1.0e-5f) return 15;
    return 215 + roundf(log10f(level) * 40.0f);
  }

  bool volumeInteger(uint32_t n) { return write(DAC_ATTENUATION_ALL, n); }

  bool volumeInteger(int channel, uint32_t n) {
    bool rv = false;

    if (0 != channel && channel <= DAC_CHANNELS_MAX)
      rv = write(DAC_ATTENUATION_BASE + channel - 1, n);

    return rv;
  }

  bool inputLevelInteger(int32_t n) {
    uint8_t levels[ADC_CHANNELS_MAX];

    for (int i = 0; i < ADC_CHANNELS_MAX; i++) levels[i] = n;

    return write(ADC_ATTENUATION_BASE, levels, ADC_CHANNELS_MAX);
  }

  bool inputLevelInteger(int channel, int32_t n) {
    bool rv = false;

    if (0 != channel && channel <= ADC_CHANNELS_MAX)
      rv = write(ADC_ATTENUATION_BASE + channel - 1, n);

    return rv;
  }

  bool write(uint32_t address, uint32_t data) {
    assert(wire != nullptr);
    // wire->beginTransmission(i2c_addr);
    // wire->write(address);
    // wire->write(data);
    // return wire->endTransmission() == 0;
    return i2c_bus_write_bytes(wire, address,(uint8_t*) &data, 4, nullptr, 0) == 0;
  }

  bool write(uint32_t address, const void *data, uint32_t len) {
    assert(wire != nullptr);
    // wire->beginTransmission(i2c_addr);
    // wire->write(address);
    // const uint8_t *p = (const uint8_t *)data;
    // wire->write(p, len);
    // return wire->endTransmission() == 0;
    return i2c_bus_write_bytes(wire, address, (uint8_t*) data, len, nullptr, 0) == 0;
  }
};
