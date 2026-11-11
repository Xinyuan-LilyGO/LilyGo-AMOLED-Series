/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in
 * which case, it is free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "es7243.h"

#include <string.h>

#include "Platforms/etc.h"

#define MCLK_PULSES_NUMBER (20)
#define ES_ASSERT(a, format, b, ...) \
  if ((a) != 0) {                    \
    AD_LOGE(format, ##__VA_ARGS__);  \
    return b;                        \
  }

static i2c_bus_handle_t i2c_handle = NULL;
static int es7243_addr = 0x13;  // 0x26>>1;
static int mclk_gpio = 0;
static int actual_volume = 0;
static uint8_t audio_format = 0x0C;

void es7243_mclk_gpio(int gpio) { mclk_gpio = gpio; }

static error_t es7243_write_reg(uint8_t reg_add, uint8_t data) {
  return i2c_bus_write_bytes(i2c_handle, es7243_addr, &reg_add, sizeof(reg_add),
                             &data, sizeof(data));
}

error_t es7243_adc_set_addr(int addr) {
  es7243_addr = addr;
  return RESULT_OK;
}

static error_t es7243_mclk_active(uint8_t mclk_gpio) {
  /*
      Before initializing es7243, it is necessary to output
      mclk to es7243 to activate the I2C configuration.
      So give some clocks to active es7243.
  */
  pinMode(mclk_gpio, OUTPUT);
  for (int i = 0; i < MCLK_PULSES_NUMBER; ++i) {
    digitalWrite(mclk_gpio, 0);
    delay(1);
    digitalWrite(mclk_gpio, 1);
    delay(1);
  }
  return RESULT_OK;
}

error_t es7243_adc_init(codec_config_t *codec_cfg, i2c_bus_handle_t handle) {
  error_t ret = RESULT_OK;
  i2c_handle = handle;
  es7243_mclk_active(mclk_gpio);
  ret |= es7243_write_reg(0x00, 0x01);  // slave mode, software mode
  ret |= es7243_write_reg(0x06, 0x00);
  ret |= es7243_write_reg(0x05, 0x1B);          // Mute ADC
  ret |= es7243_write_reg(0x01, audio_format);  // i2s -16bit
  ret |= es7243_write_reg(0x08, 0x43);          // enable AIN, PGA GAIN = 27DB
  ret |= es7243_write_reg(0x05, 0x13);          // un Mute ADC
  if (ret) {
    AD_LOGE("Es7243 initialize failed!");
    return RESULT_FAIL;
  }
  return ret;
}

error_t es7243_adc_deinit(void) { return es7243_adc_set_voice_mute(true); }

error_t es7243_adc_ctrl_state_active(codec_mode_t mode,
                                     bool ctrl_state_active) {
  return es7243_adc_set_voice_mute(!ctrl_state_active);
}

error_t es7243_adc_config_i2s(codec_mode_t mode, I2SDefinition *iface) {
  // master mode not supported
  if (iface->mode == MODE_MASTER) {
    AD_LOGE("es7243_adc_config_i2s: Mode must be slave");
    return RESULT_FAIL;
  }

  // set bits
  switch (iface->bits) {
    case BIT_LENGTH_16BITS:
      audio_format |= 0b011 < 2;
      break;
    case BIT_LENGTH_24BITS:
      audio_format |= 0b000 < 2;
      break;
    case BIT_LENGTH_32BITS:
      audio_format |= 0b100 < 2;
      break;
    default:
      AD_LOGE("es7243_adc_config_i2s: bits not supported %d", iface->bits);
      return RESULT_FAIL;
  }

  switch (iface->fmt) {
    case I2S_RIGHT:
    case I2S_NORMAL:
      audio_format |= 0b00;
      break;
    case I2S_LEFT:
      audio_format |= 0b01;
      break;
    case I2S_DSP:
      audio_format |= 0b11;
      break;
    default:
      AD_LOGE("es7243_adc_config_i2s: mft not supported %d", iface->fmt);
      return RESULT_FAIL;
  }

  error_t ret = es7243_write_reg(0x01, audio_format);
  return ret;
}

error_t es7243_adc_set_voice_mute(bool mute) {
  AD_LOGI("es7243_adc_set_voice_mute es7243_mute(), mute = %d", mute);
  error_t ret = RESULT_OK;
  if (mute) {
    ret = es7243_write_reg(0x05, 0x1B);
  } else {
    ret = es7243_write_reg(0x05, 0x13);
  }
  return ret;
}

error_t es7243_adc_set_voice_volume(int volume) {
  AD_LOGI("es7243_adc_set_voice_volume: %d", volume);
  error_t ret = RESULT_OK;
  if (volume > 100) {
    volume = 100;
  }
  if (volume < 0) {
    volume = 0;
  }
  actual_volume = volume;
  switch (volume) {
    case 0 ... 12:
      ret |= es7243_write_reg(0x08, 0x11);  // 1db
      break;
    case 13 ... 25:
      ret |= es7243_write_reg(0x08, 0x13);  // 3.5db
      break;
    case 26 ... 38:
      ret |= es7243_write_reg(0x08, 0x21);  // 18db
      break;
    case 39 ... 51:
      ret |= es7243_write_reg(0x08, 0x23);  // 20.5db
      break;
    case 52 ... 65:
      ret |= es7243_write_reg(0x08, 0x05);  // 22.5db
      break;
    case 66 ... 80:
      ret |= es7243_write_reg(0x08, 0x41);  // 24.5db
      break;
    case 81 ... 90:
      ret |= es7243_write_reg(0x08, 0x07);  // 25db
      break;
    case 91 ... 100:
      ret |= es7243_write_reg(0x08, 0x43);  // 27db
      break;
    default:
      break;
  }
  return ret;
}

error_t es7243_adc_get_voice_volume(int *volume) {
  *volume = actual_volume;
  return RESULT_OK;
}
