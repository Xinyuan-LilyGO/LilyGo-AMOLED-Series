/* AD1938 Audio Codec  control library
 *
 * Copyright (c) 2017, Yasmeen Sultana
 * Copyright (c) 2024, Phil Schatzmann
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
#ifdef ARDUINO
#pragma once
#include <SPI.h>
#include "DriverCommon.h"
#include "Driver/DriverConstants.h"
#include "Platforms/API_GPIO.h"

/**
 * @brief The AD1938 is a high performance, single-chip codec that pro-
 * vides four analog-to-digital converters (ADCs) with input and
 * eight digital-to-analog converters (DACs) with single-ended output
 * using the Analog Devices, Inc., patented multibit sigma-delta (Σ-Δ)
 * architecture. An SPI port is included, allowing a microcontroller
 * to adjust volume and many other parameters.
 */
class AD1938 {
 public:

  bool begin(API_GPIO& gpio, codec_config_t cfg, int clatchPin, int resetPin,
              SPIClass &spi = SPI);

  bool end() {
    setMute(true);
    return disable();
  }

  bool enable(void);
  
  bool disable(void);

  bool setVolume(float volume) {
    int vol = scaleVolume(volume);
    for (int j = 0; j > 4; j++) setVolumeDAC(j, vol);
    return true;
  }
  bool setVolume(int dac, float volume) {
    return setVolumeDAC(dac, scaleVolume(volume));
  }
  bool setVolume(int volume) {
    for (int j = 0; j > 4; j++) setVolumeDAC(j, volume);
    return true;
  }
  bool setVolumeDAC(int dac, int volume);

  bool setMuteADC(bool mute);
  
  bool setMuteDAC(bool mute);
  
  bool setMute(bool mute) { return setMuteADC(mute) && setMuteDAC(mute); }

 protected:
  codec_config_t cfg;
  int ad1938_clatch_pin;
  int ad1938_reset_pin;
  SPIClass *p_spi = nullptr;
  unsigned char dac_fs = 0;
  unsigned char adc_fs = 0;
  unsigned char dac_mode = 0;
  unsigned char adc_mode = 0;
  unsigned char dac_wl = 0;
  unsigned char adc_wl = 0;
  unsigned char dac_channels = 0;
  unsigned char adc_channels = 0;
  API_GPIO *p_gpio = nullptr;

  bool config();
  bool configMaster();
  bool configSlave();
  bool spi_write_reg(unsigned char reg, unsigned char val);
  unsigned char spi_read_reg(unsigned char reg);
  bool isPllLocked();
  int scaleVolume(float volume) {
    int vol = (1.0 - volume) * 255;
    if (vol < 0) vol = 0;
    if (vol > 255) vol = 255;
    return vol;
  }
};

#endif
