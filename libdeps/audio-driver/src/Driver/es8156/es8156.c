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

#include "string.h"
#include <math.h>
#include "es8156.h"

#ifndef BIT
#define BIT(nr)             (1 << (nr))
#endif
// 0x8 = 0x10>>1
#define ES8156_ADDR         0x8
#define BOARD_PA_GAIN       (0) /* Power amplifier gain defined by board (dB) */

//static const char *TAG = "DRV8156";
static bool codec_init_flag = 0;
static i2c_bus_handle_t i2c_handle = NULL;
static codec_dac_volume_config_t *dac_vol_handle;

#define ES8156_DAC_VOL_CFG_DEFAULT() {                      \
    .max_dac_volume = 32,                                   \
    .min_dac_volume = -95.5,                                \
    .board_pa_gain = BOARD_PA_GAIN,                         \
    .volume_accuracy = 0.5,                                 \
    .dac_vol_symbol = 1,                                    \
    .zero_volume_reg = 0xBF,                                \
    .reg_value = 0,                                         \
    .user_volume = 0,                                       \
    .offset_conv_volume = NULL,                             \
}

// func_t AUDIO_CODEC_ES8156_DEFAULT_HANDLE = {
//     .audio_codec_initialize = es8156_codec_init,
//     .audio_codec_deinitialize = es8156_codec_deinit,
//     .audio_codec_ctrl = es8156_codec_ctrl_state_active,
//     .audio_codec_config_iface = es8156_codec_config_i2s,
//     .audio_codec_set_mute = es8156_codec_set_voice_mute,
//     .audio_codec_set_volume = es8156_codec_set_voice_volume,
//     .audio_codec_get_volume = es8156_codec_get_voice_volume,
//     .lock = NULL,
//     .handle = NULL,
// };

static bool es8156_codec_initialized()
{
    return codec_init_flag;
}

static error_t es8156_write_reg(uint8_t reg_addr, uint8_t data)
{
    return i2c_bus_write_bytes(i2c_handle, ES8156_ADDR, &reg_addr, sizeof(reg_addr), &data, sizeof(data));
}

static int es8156_read_reg(uint8_t reg_addr)
{
    uint8_t data;
    i2c_bus_read_bytes(i2c_handle, ES8156_ADDR, &reg_addr, sizeof(reg_addr), &data, sizeof(data));
    return (int)data;
}


static error_t es8156_standby(void)
{
    error_t ret = 0;
    ret = es8156_write_reg(0x14, 0x00);
    ret |= es8156_write_reg(0x19, 0x02);
    ret |= es8156_write_reg(0x21, 0x1F);
    ret |= es8156_write_reg(0x22, 0x02);
    ret |= es8156_write_reg(0x25, 0x21);
    ret |= es8156_write_reg(0x25, 0xA1);
    ret |= es8156_write_reg(0x18, 0x01);
    ret |= es8156_write_reg(0x09, 0x02);
    ret |= es8156_write_reg(0x09, 0x01);
    ret |= es8156_write_reg(0x08, 0x00);
    return ret;
}

static error_t es8156_resume(void)
{
    error_t ret = 0;
    ret |= es8156_write_reg(0x08, 0x3F);
    ret |= es8156_write_reg(0x09, 0x00);
    ret |= es8156_write_reg(0x18, 0x00);

    ret |= es8156_write_reg(0x25, 0x20);
    ret |= es8156_write_reg(0x22, 0x00);
    ret |= es8156_write_reg(0x21, 0x3C);
    ret |= es8156_write_reg(0x19, 0x20);
    ret |= es8156_write_reg(0x14, 179);
    return ret;
}

// void es8156_pa_power(bool enable)
// {
//     if (enable) {
//         gpio_set_level(get_pa_enable_gpio(), 1);
//     } else {
//         gpio_set_level(get_pa_enable_gpio(), 0);
//     }
// }

error_t es8156_codec_init(codec_config_t *cfg, void* i2c)
{
    i2c_handle = i2c;
    if (es8156_codec_initialized()) {
        AD_LOGW("The es8156 DAC has been already initialized");
        return RESULT_OK;
    }
    codec_init_flag = true;

    es8156_write_reg(0x02, 0x04);
    es8156_write_reg(0x20, 0x2A);
    es8156_write_reg(0x21, 0x3C);
    es8156_write_reg(0x22, 0x00);
    es8156_write_reg(0x24, 0x07);
    es8156_write_reg(0x23, 0x00);

    es8156_write_reg(0x0A, 0x01);
    es8156_write_reg(0x0B, 0x01);
    es8156_write_reg(0x11, 0x00);
    es8156_write_reg(0x14, 179);  // volume 70%

    es8156_write_reg(0x0D, 0x14);
    es8156_write_reg(0x18, 0x00);
    es8156_write_reg(0x08, 0x3F);
    es8156_write_reg(0x00, 0x02);
    es8156_write_reg(0x00, 0x03);
    es8156_write_reg(0x25, 0x20);

    // gpio_config_t io_conf;
    // memset(&io_conf, 0, sizeof(io_conf));
    // io_conf.mode = GPIO_MODE_OUTPUT;
    // io_conf.pin_bit_mask = BIT64(get_pa_enable_gpio());
    // io_conf.pull_down_en = 0;
    // io_conf.pull_up_en = 0;
    // gpio_config(&io_conf);
    //es8156_pa_power(true);

    // codec_dac_volume_config_t vol_cfg = ES8156_DAC_VOL_CFG_DEFAULT();
    // dac_vol_handle = audio_codec_volume_init(&vol_cfg);
    return RESULT_OK;
}

error_t es8156_codec_deinit(void)
{
    codec_init_flag = false;
    //audio_codec_volume_deinit(dac_vol_handle);
    return RESULT_OK;
}

error_t es8156_codec_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active)
{
    error_t ret = RESULT_OK;
    if (ctrl_state_active) {
        ret = es8156_resume();
    } else {
        AD_LOGW("The codec going to stop");
        ret = es8156_standby();
    }
    return ret;
}

error_t es8156_codec_config_i2s(codec_mode_t mode, I2SDefinition *iface)
{
    return RESULT_OK;
}

error_t es8156_codec_set_voice_mute(bool enable)
{
    int regv = es8156_read_reg(ES8156_DAC_MUTE_REG13);
    if (enable) {
        regv = regv | BIT(1) | BIT(2);
    } else {
        regv = regv & (~(BIT(1) | BIT(2))) ;
    }
    es8156_write_reg(ES8156_DAC_MUTE_REG13, regv);
    return RESULT_OK;
}

/**
 * @brief Set voice volume
 *
 * @note Register values. 0x00: -95.5 dB, 0x5B: -50 dB, 0xBF: 0 dB, 0xFF: 32 dB
 * @note Accuracy of gain is 0.5 dB
 *
 * @param volume: voice volume (0~100)
 *
 * @return
 *     - RESULT_OK
 *     - RESULT_FAIL
 */
error_t es8156_codec_set_voice_volume(int volume)
{
    int ret = 0;
    // TODO
    // uint8_t reg = 0;
    // reg = audio_codec_get_dac_reg_value(dac_vol_handle, volume);
    // ret = es8156_write_reg(ES8156_VOLUME_CONTROL_REG14, reg);
    // AD_LOGD( "Set volume:%.2d reg_value:0x%.2x dB:%.1f", dac_vol_handle->user_volume, reg,
    //         audio_codec_cal_dac_volume(dac_vol_handle));
    return ret;
}

error_t es8156_codec_get_voice_volume(int *volume)
{
    int ret = 0;
    int regv = 0;
    *volume = 0;
    regv = es8156_read_reg(ES8156_VOLUME_CONTROL_REG14);
    if (regv == RESULT_FAIL) {
        ret = RESULT_FAIL;
    } else {
        if (regv == dac_vol_handle->reg_value) {
            *volume = dac_vol_handle->user_volume;
        } else {
            *volume = 0;
            ret = RESULT_FAIL;
        }
    }
    AD_LOGD("Get volume:%.2d reg_value:0x%.2x", *volume, regv);
    return ret;
}
