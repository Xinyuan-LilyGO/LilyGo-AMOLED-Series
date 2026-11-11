/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#pragma once

#include "stdbool.h"
#include "DriverCommon.h"
#include "Driver/DriverConstants.h"
#include "Platforms/API_I2C.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ES8388 address  0x22:CE=1;0x20:CE=0 */
#define ES8388_ADDR 0x10  
/* ES8388 register */
#define ES8388_CONTROL1         0x00
#define ES8388_CONTROL2         0x01

#define ES8388_CHIPPOWER        0x02

#define ES8388_ADCPOWER         0x03
#define ES8388_DACPOWER         0x04

#define ES8388_CHIPLOPOW1       0x05
#define ES8388_CHIPLOPOW2       0x06

#define ES8388_ANAVOLMANAG      0x07

#define ES8388_MASTERMODE       0x08
/* ADC */
#define ES8388_ADCCONTROL1      0x09
#define ES8388_ADCCONTROL2      0x0a
#define ES8388_ADCCONTROL3      0x0b
#define ES8388_ADCCONTROL4      0x0c
#define ES8388_ADCCONTROL5      0x0d
#define ES8388_ADCCONTROL6      0x0e
#define ES8388_ADCCONTROL7      0x0f
#define ES8388_ADCCONTROL8      0x10
#define ES8388_ADCCONTROL9      0x11
#define ES8388_ADCCONTROL10     0x12
#define ES8388_ADCCONTROL11     0x13
#define ES8388_ADCCONTROL12     0x14
#define ES8388_ADCCONTROL13     0x15
#define ES8388_ADCCONTROL14     0x16
/* DAC */
#define ES8388_DACCONTROL1      0x17
#define ES8388_DACCONTROL2      0x18
#define ES8388_DACCONTROL3      0x19
#define ES8388_DACCONTROL4      0x1a
#define ES8388_DACCONTROL5      0x1b
#define ES8388_DACCONTROL6      0x1c
#define ES8388_DACCONTROL7      0x1d
#define ES8388_DACCONTROL8      0x1e
#define ES8388_DACCONTROL9      0x1f
#define ES8388_DACCONTROL10     0x20
#define ES8388_DACCONTROL11     0x21
#define ES8388_DACCONTROL12     0x22
#define ES8388_DACCONTROL13     0x23
#define ES8388_DACCONTROL14     0x24
#define ES8388_DACCONTROL15     0x25
#define ES8388_DACCONTROL16     0x26
#define ES8388_DACCONTROL17     0x27
#define ES8388_DACCONTROL18     0x28
#define ES8388_DACCONTROL19     0x29
#define ES8388_DACCONTROL20     0x2a
#define ES8388_DACCONTROL21     0x2b
#define ES8388_DACCONTROL22     0x2c
#define ES8388_DACCONTROL23     0x2d
#define ES8388_DACCONTROL24     0x2e
#define ES8388_DACCONTROL25     0x2f
#define ES8388_DACCONTROL26     0x30
#define ES8388_DACCONTROL27     0x31
#define ES8388_DACCONTROL28     0x32
#define ES8388_DACCONTROL29     0x33
#define ES8388_DACCONTROL30     0x34

typedef enum {
  ESP8388_INPUT_MIN = -1,
  ESP8388_INPUT_LINPUT1_RINPUT1 = 0x00,
  ESP8388_INPUT_MIC1 = 0x05,
  ESP8388_INPUT_MIC2 = 0x06,
  ESP8388_INPUT_LINPUT2_RINPUT2 = 0x50,
  ESP8388_INPUT_DIFFERENCE = 0xf0,
  ESP8388_INPUT_MAX,
} es8388_input_device_t;

/**
 * @brief Initialize ES8388 codec chip
 *
 * @param cfg configuration of ES8388
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_init(codec_config_t *cfg, i2c_bus_handle_t handle, int addr, int volumeHack);

/**
 * @brief Deinitialize ES8388 codec chip
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_deinit(void);

/**
 * @brief Configure ES8388 I2S format
 *
 * @param mod:  set ADC or DAC or both
 * @param cfg:   ES8388 I2S format
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_config_fmt(codec_mode_t mod, i2s_format_t cfg);

/**
 * @brief Configure I2s clock in MSATER mode
 *
 * @param cfg:  set bits clock and WS clock
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_i2s_config_clock(es_i2s_clock_t cfg);

/**
 * @brief Configure ES8388 data sample bits
 *
 * @param mode:  set ADC or DAC or both
 * @param bit_per_sample:  bit number of per sample
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_set_bits_per_sample(codec_mode_t mode, sample_bits_t bit_per_sample);

/**
 * @brief  Start ES8388 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_start(codec_mode_t mode);

/**
 * @brief  Stop ES8388 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_stop(codec_mode_t mode);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_set_voice_volume(int volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8388_get_voice_volume(int *volume);

/**
 * @brief Configure ES8388 DAC mute or not. Basically you can use this function to mute the output or unmute
 *
 * @param enable enable(1) or disable(0)
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_set_voice_mute(bool enable);

/**
 * @brief Get ES8388 DAC mute status
 *
 *  @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_get_voice_mute(void);

/**
 * @brief Set ES8388 mic gain
 *
 * @param gain db of mic gain
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_set_mic_gain(es_mic_gain_t gain);

/**
 * @brief Set ES8388 adc input mode
 *
 * @param input adc input mode
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_config_input_device(es8388_input_device_t input);

/**
 * @brief Set ES8388 dac output mode
 *
 * @param output dac output mode
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_config_output_device(output_device_t output);

/**
 * @brief Write ES8388 register
 *
 * @param reg_add address of register
 * @param data data of register
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_write_reg(uint8_t reg_add, uint8_t data);

/**
 * @brief Reads ES8388 register
 *
 * @param reg_add address of register
 * @param data data of register
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */

error_t es8388_read_reg(uint8_t reg_add, uint8_t *p_data);

/**
 * @brief Print all ES8388 registers
 *
 * @return
 *     - void
 */
void es8388_read_all();

/**
 * @brief Configure ES8388 codec mode and I2S interface
 *
 * @param mode codec mode
 * @param iface I2S config
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_config_i2s(codec_mode_t mode, I2SDefinition *iface);

/**
 * @brief Control ES8388 codec chip
 *
 * @param mode codec mode
 * @param ctrl_state_active start or stop decode or encode progress
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8388_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active);


#ifdef __cplusplus
}
#endif

