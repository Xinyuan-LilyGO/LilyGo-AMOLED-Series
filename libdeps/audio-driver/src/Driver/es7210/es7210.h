/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2021 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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

#define  ES7210_RESET_REG00                 0x00        /* Reset control */
#define  ES7210_CLOCK_OFF_REG01             0x01        /* Used to turn off the ADC clock */
#define  ES7210_MAINCLK_REG02               0x02        /* Set ADC clock frequency division */
#define  ES7210_MASTER_CLK_REG03            0x03        /* MCLK source $ SCLK division */
#define  ES7210_LRCK_DIVH_REG04             0x04        /* lrck_divh */
#define  ES7210_LRCK_DIVL_REG05             0x05        /* lrck_divl */
#define  ES7210_POWER_DOWN_REG06            0x06        /* power down */
#define  ES7210_OSR_REG07                   0x07
#define  ES7210_MODE_CONFIG_REG08           0x08        /* Set master/slave & channels */
#define  ES7210_TIME_CONTROL0_REG09         0x09        /* Set Chip intial state period*/
#define  ES7210_TIME_CONTROL1_REG0A         0x0A        /* Set Power up state period */
#define  ES7210_SDP_INTERFACE1_REG11        0x11        /* Set sample & fmt */
#define  ES7210_SDP_INTERFACE2_REG12        0x12        /* Pins state */
#define  ES7210_ADC_AUTOMUTE_REG13          0x13        /* Set mute */
#define  ES7210_ADC34_MUTERANGE_REG14       0x14        /* Set mute range */
#define  ES7210_ADC34_HPF2_REG20            0x20        /* HPF */
#define  ES7210_ADC34_HPF1_REG21            0x21
#define  ES7210_ADC12_HPF1_REG22            0x22
#define  ES7210_ADC12_HPF2_REG23            0x23
#define  ES7210_ANALOG_REG40                0x40        /* ANALOG Power */
#define  ES7210_MIC12_BIAS_REG41            0x41
#define  ES7210_MIC34_BIAS_REG42            0x42
#define  ES7210_MIC1_ES7210_GAIN_REG43             0x43
#define  ES7210_MIC2_ES7210_GAIN_REG44             0x44
#define  ES7210_MIC3_ES7210_GAIN_REG45             0x45
#define  ES7210_MIC4_ES7210_GAIN_REG46             0x46
#define  ES7210_MIC1_POWER_REG47            0x47
#define  ES7210_MIC2_POWER_REG48            0x48
#define  ES7210_MIC3_POWER_REG49            0x49
#define  ES7210_MIC4_POWER_REG4A            0x4A
#define  ES7210_MIC12_POWER_REG4B           0x4B        /* MICBias & ADC & PGA Power */
#define  ES7210_MIC34_POWER_REG4C           0x4C

typedef enum {
    ES7210_AD1_AD0_00 = 0x80,
    ES7210_AD1_AD0_01 = 0x82,
    ES7210_AD1_AD0_10 = 0x84,
    ES7210_AD1_AD0_11 = 0x86
} es7210_address_t;

typedef enum {
    ES7210_INPUT_MIC1 = 0x01,
    ES7210_INPUT_MIC2 = 0x02,
    ES7210_INPUT_MIC3 = 0x04,
    ES7210_INPUT_MIC4 = 0x08
} es7210_input_mics_t;

typedef enum gain_value{
	ES7210_GAIN_0DB = 0,
	ES7210_GAIN_3DB,
	ES7210_GAIN_6DB,
	ES7210_GAIN_9DB,
	ES7210_GAIN_12DB,
	ES7210_GAIN_15DB,
	ES7210_GAIN_18DB,
	ES7210_GAIN_21DB,
	ES7210_GAIN_24DB,
	ES7210_GAIN_27DB,
	ES7210_GAIN_30DB,
	ES7210_GAIN_33DB,
	ES7210_GAIN_34_5DB,
	ES7210_GAIN_36DB,
	ES7210_GAIN_37_5DB,
} es7210_gain_value_t;

/*
 * @brief Initialize ES7210 ADC chip
 *
 * @param[in] codec_cfg:  configuration of ES7210
 *
 * @return
 *      - RESULT_OK
 *      - RESULT_FAIL
 */
error_t es7210_adc_init(codec_config_t *codec_cfg, void* i2c);

/**
 * @brief Deinitialize ES7210 ADC chip
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7210_adc_deinit();

/**
 * @brief Configure ES7210 ADC mode and I2S interface
 *
 * @param[in] mode:  codec mode
 * @param[in] iface:  I2S config
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es7210_adc_config_i2s(codec_mode_t mode, I2SDefinition *iface);

/**
 * @brief Control ES7210 ADC chip
 *
 * @param[in] mode:  codec mode
 * @param[in] ctrl_state_active:  start or stop progress
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es7210_adc_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active);

/**
 * @brief  Set gain (Note: the enabled microphone sets the same gain)
 *
 * @param[in] gain:  gain 
 * 
 *       gain        :   value
 *       ES7210_GAIN_0DB    :   1
 *       ES7210_GAIN_3DB    :   2
 *       ES7210_GAIN_6DB    :   3
 *           ·
 *           ·
 *           ·
 *       ES7210_GAIN_30DB   :   10
 *       ES7210_GAIN_33DB   :   11
 *       ES7210_GAIN_34_5DB :   12
 *       ES7210_GAIN_36DB   :   13
 *       ES7210_GAIN_37_5DB :   14
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7210_adc_set_gain(es7210_gain_value_t gain);

/**
 * @brief Get gain
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7210_adc_get_gain(void);

/**
 * @brief Set volume
 *
 * @param[in] volume:  volume
 *
 * @return
 *     - RESULT_OK
 */
error_t es7210_adc_set_volume(int volume);

/**
 * @brief Set ES7210 ADC mute status
 *
 * @return
 *     - RESULT_FAIL
 *     - RESULT_OK
 */
error_t es7210_set_mute(bool enable);

/**
 * @brief Select ES7210 mic
 *
 * @param[in] mic:  mics
 * 
 * @return
 *     - RESULT_FAIL
 *     - RESULT_OK
 */
error_t es7210_mic_select(es7210_input_mics_t mic);

/**
 * @brief Read regs of ES7210
 *
 * @param[in] reg_addr:  reg_addr
 * 
 * @return
 *     - RESULT_FAIL
 *     - RESULT_OK
 */
int es7210_read_reg(uint8_t reg_addr);

/**
 * @brief Read all regs of ES7210
 */
void es7210_read_all(void);

#ifdef __cplusplus
}
#endif

