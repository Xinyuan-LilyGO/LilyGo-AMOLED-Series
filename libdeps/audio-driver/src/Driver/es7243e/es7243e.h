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

#ifndef _ES7243E_H_
#define _ES7243E_H_

#include "stdbool.h"
#include "DriverCommon.h"
#include "Platforms/API_I2C.h"
#include "Driver/DriverConstants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ES7243E adc chip
 *
 * @param codec_cfg configuration of ES7243E
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7243e_adc_init(codec_config_t *codec_cfg, void* i2c);

/**
 * @brief Deinitialize ES7243E adc chip
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7243e_adc_deinit(void);

/**
 * @brief Control ES7243E adc chip
 *
 * @param mode adc mode
 * @param ctrl_state_active start or stop decode or encode progress
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es7243e_adc_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active);

/**
 * @brief Configure ES7243E adc mode and I2S interface
 *
 * @param mode codec mode
 * @param iface I2S config
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t es7243e_adc_config_i2s(codec_mode_t mode, I2SDefinition *iface);

/**
 * @brief  Set adc gain
 *
 * @param volume  value of gain (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7243e_adc_set_voice_volume(int volume);

/**
 * @brief Get adc gain
 *
 * @param[out] *volume:  value of gain (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7243e_adc_get_voice_volume(int *volume);

/**
 * @brief Set adc I2C address
 *
 * @param[in] addr:  value of I2C address
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es7243e_adc_set_addr(int addr);

#ifdef __cplusplus
}
#endif

#endif
