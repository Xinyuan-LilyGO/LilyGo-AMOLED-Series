/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2020 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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

#define TAS5805M_REG_00      0x00
#define TAS5805M_REG_02      0x02
#define TAS5805M_REG_03      0x03
#define TAS5805M_REG_24      0x24
#define TAS5805M_REG_25      0x25
#define TAS5805M_REG_26      0x26
#define TAS5805M_REG_27      0x27
#define TAS5805M_REG_28      0x28
#define TAS5805M_REG_29      0x29
#define TAS5805M_REG_2A      0x2a
#define TAS5805M_REG_2B      0x2b
#define TAS5805M_REG_35      0x35
#define TAS5805M_REG_7E      0x7e
#define TAS5805M_REG_7F      0x7f

#define TAS5805M_PAGE_00     0x00
#define TAS5805M_PAGE_2A     0x2a

#define TAS5805M_BOOK_00     0x00
#define TAS5805M_BOOK_8C     0x8c

#define  MASTER_VOL_REG_ADDR    0X4C
#define  MUTE_TIME_REG_ADDR     0X51

#define  TAS5805M_DAMP_MODE_BTL      0x0
#define  TAS5805M_DAMP_MODE_PBTL     0x04


void tas5805m_set_power_pin(int pin);

/**
 * @brief Initialize TAS5805 codec chip
 *
 * @param cfg configuration of TAS5805
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t tas5805m_init(codec_config_t *codec_cfg, void* i2c);

/**
 * @brief Deinitialize TAS5805 codec chip
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t tas5805m_deinit(void);

/**
 * @brief  Set voice volume
 *
 * @param volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t tas5805m_set_volume(int vol);

/**
 * @brief Get voice volume
 *
 * @param[out] *volume:  voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t tas5805m_get_volume(int *value);

/**
 * @brief Set TAS5805 mute or not
 *        Continuously call should have an interval time determined by tas5805m_set_mute_fade()
 *
 * @param enable enable(1) or disable(0)
 *
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t tas5805m_set_mute(bool enable);

/**
 * @brief Mute gradually by (value)ms
 *
 * @param value  Time for mute with millisecond.
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 *
 */
error_t tas5805m_set_mute_fade(int value);

/**
 * @brief Get TAS5805 mute status
 *
 *  @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 */
error_t tas5805m_get_mute(int *value);

/**
 * @brief Set DAMP mode
 *
 * @param value  TAS5805M_DAMP_MODE_BTL or TAS5805M_DAMP_MODE_PBTL
 * @return
 *     - RESULT_FAIL Parameter error
 *     - RESULT_OK   Success
 *
 */
error_t tas5805m_set_damp_mode(int value);

#ifdef __cplusplus
}
#endif

