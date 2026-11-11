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
#include "Platforms/API_I2C.h"
#include "Driver/DriverConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ES8374 address 0x22:CE=1;0x20:CE=0 / 0x20>>1 = 0x10 */
#define ES8374_ADDR 0x10 

/**
 * @brief Initialize ES8374 codec chip
 *
 * @param cfg configuration of ES8374
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_codec_init(codec_config_t *cfg, codec_mode_t codec_mode, void *i2c, int i2c_addr);

/**
 * @brief Deinitialize ES8374 codec chip
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_codec_deinit(void);

/**
 * @brief Configure ES8374 I2S format
 *
 * @param mode:  set ADC or DAC or both
 * @param fmt:  ES8374 I2S format
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_config_fmt(codec_mode_t mode, i2s_format_t fmt);

/**
 * @brief Configure I2S clock in MSATER mode
 *
 * @param cfg:  set bits clock and WS clock
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_i2s_config_clock(es_i2s_clock_t cfg);

/**
 * @brief Configure ES8374 data sample bits
 *
 * @param mode:  set ADC or DAC or both
 * @param bit_per_sample:  bit number of per sample
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_set_bits_per_sample(codec_mode_t mode, sample_bits_t bit_per_sample);

/**
 * @brief  Start ES8374 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_start(codec_mode_t mode);

/**
 * @brief  Stop ES8374 codec chip
 *
 * @param mode:  set ADC or DAC or both
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_stop(codec_mode_t mode);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_codec_set_voice_volume(int volume);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8374_codec_get_voice_volume(int *volume);

/**
 * @brief Mute or unmute ES8374 DAC. Basically you can use this function to mute or unmute the output
 *
 * @param enable mute(1) or unmute(0)
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_set_voice_mute(bool enable);

/**
 * @brief Get ES8374 DAC mute status
 *
 * @return
 *     - RESULT_FAIL
 *     - RESULT_OK
 */
error_t es8374_get_voice_mute(void);

/**
 * @brief Set ES8374 mic gain
 *
 * @param gain db of mic gain
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_set_mic_gain(es_mic_gain_t gain);

/**
 * @brief Set ES8374 ADC input mode
 *
 * @param input adc input mode
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_config_input_device();

/**
 * @brief Set ES8374 DAC output mode
 *
 * @param output dac output mode
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_config_output_device();

/**
 * @brief Write ES8374 register
 *
 * @param reg_add address of register
 * @param data data of register
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_write_reg(uint8_t reg_add, uint8_t data);

/**
 * @brief Print all ES8374 registers
 *
 * @return
 *    - void
 */
void es8374_read_all();

/**
 * @brief Configure ES8374 codec mode and I2S interface
 *
 * @param mode codec mode
 * @param iface I2S config
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_codec_config_i2s(codec_mode_t mode, I2SDefinition *iface);

/**
 * @brief Control ES8374 codec chip
 *
 * @param mode codec mode
 * @param ctrl_state_active start or stop decode or encode progress
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es8374_codec_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active);

typedef enum {
    ES8374_PGA_GAIN_MIN = -1,
    ES8374_PGA_GAIN_DIS = 0,
    ES8374_PGA_GAIN_EN = 1,
    ES8374_PGA_GAIN_MAX = 2,
} es_d2se_pga_t;


#ifdef __cplusplus
}
#endif


