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

#include <string.h>
#include "es8374.h"
//#include "audio_hal/board_pins_config.h"

#define ES_ASSERT(a, format, b, ...) \
    if ((a) != 0) { \
        AD_LOGE(format, ##__VA_ARGS__); \
        return b;\
    }


static int codec_init_flag = 0;
static i2c_bus_handle_t i2c_handle = NULL;
static int i2c_address_es8374 = ES8374_ADDR;


static bool es8374_codec_initialized()
{
    return codec_init_flag;
}

static error_t es_write_reg(uint8_t slave_addr, uint8_t reg_add, uint8_t data)
{
    return i2c_bus_write_bytes(i2c_handle, slave_addr, &reg_add, sizeof(reg_add), &data, sizeof(data));
}

static error_t es_read_reg(uint8_t slave_addr, uint8_t reg_add, uint8_t *p_data)
{
    return i2c_bus_read_bytes(i2c_handle, slave_addr, &reg_add, sizeof(reg_add), p_data, 1);
}


error_t es8374_write_reg(uint8_t reg_add, uint8_t data)
{
    return es_write_reg(i2c_address_es8374, reg_add, data);
}

int es8374_read_reg(uint8_t reg_add, uint8_t *regv)
{
    uint8_t regdata = 0xFF;
    uint8_t res = 0;

    if (es_read_reg(i2c_address_es8374, reg_add, &regdata) == 0) {
        *regv = regdata;
        return res;
    } else {
        AD_LOGI("Read Audio Codec Register Failed!");
        res = -1;
        return res;
    }
}

void es8374_read_all()
{
    for (int i = 0; i < 50; i++) {
        uint8_t reg = 0;
        es8374_read_reg(i, &reg);
        AD_LOGI( "%x: %x", i, reg);
    }
}

error_t es8374_set_voice_mute(bool enable)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    res |= es8374_read_reg(0x36, &reg);
    if (res == 0) {
        reg = reg & 0xdf;
        res |= es8374_write_reg(0x36, reg | (((int)enable) << 5));
    }

    return res;
}

error_t es8374_get_voice_mute(void)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    res |= es8374_read_reg(0x36, &reg);
    if (res == RESULT_OK) {
        reg = reg & 0x40;
    }

    return res == RESULT_OK ? reg : res;
}

error_t es8374_set_bits_per_sample(codec_mode_t mode, sample_bits_t bit_per_sample)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;
    int bits = (int)bit_per_sample & 0x0f;

    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_read_reg(0x10, &reg);
        if (res == 0) {
            reg = reg & 0xe3;
            res |=  es8374_write_reg(0x10, reg | (bits << 2));
        }
    }
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_read_reg(0x11, &reg);
        if (res == 0) {
            reg = reg & 0xe3;
            res |= es8374_write_reg(0x11, reg | (bits << 2));
        }
    }

    return res;
}

error_t es8374_config_fmt(codec_mode_t mode, i2s_format_t fmt)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;
    int fmt_tmp, fmt_i2s;

    fmt_tmp = ((fmt & 0xf0) >> 4);
    fmt_i2s =  fmt & 0x0f;
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_read_reg(0x10, &reg);
        if (res == 0) {
            reg = reg & 0xfc;
            res |= es8374_write_reg(0x10, reg | fmt_i2s);
            res |= es8374_set_bits_per_sample(mode, fmt_tmp);
        }
    }
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_read_reg(0x11, &reg);
        if (res == 0) {
            reg = reg & 0xfc;
            res |= es8374_write_reg(0x11, reg | (fmt_i2s));
            res |= es8374_set_bits_per_sample(mode, fmt_tmp);
        }
    }

    return res;
}

error_t es8374_start(codec_mode_t mode)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    if (mode == CODEC_MODE_LINE_IN) {
        res |= es8374_read_reg(0x1a, &reg);       //set monomixer
        reg |= 0x60;
        reg |= 0x20;
        reg &= 0xf7;
        res |= es8374_write_reg( 0x1a, reg);
        res |= es8374_read_reg(0x1c, &reg);        // set spk mixer
        reg |= 0x40;
        res |= es8374_write_reg( 0x1c, reg);
        res |= es8374_write_reg(0x1D, 0x02);      // spk set
        res |= es8374_write_reg(0x1F, 0x00);      // spk set
        res |= es8374_write_reg(0x1E, 0xA0);      // spk on
    }
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH || mode == CODEC_MODE_LINE_IN) {
        res |= es8374_read_reg(0x21, &reg);       //power up adc and input
        reg &= 0x3f;
        res |= es8374_write_reg(0x21, reg);
        res |= es8374_read_reg(0x10, &reg);       //power up adc and input
        reg &= 0x3f;
        res |= es8374_write_reg(0x10, reg);
    }

    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH || mode == CODEC_MODE_LINE_IN) {
        res |= es8374_read_reg(0x1a, &reg);       //disable lout
        reg |= 0x08;
        res |= es8374_write_reg( 0x1a, reg);
        reg &= 0xdf;
        res |= es8374_write_reg( 0x1a, reg);
        res |= es8374_write_reg(0x1D, 0x12);      // mute speaker
        res |= es8374_write_reg(0x1E, 0x20);      // disable class d
        res |= es8374_read_reg(0x15, &reg);        //power up dac
        reg &= 0xdf;
        res |= es8374_write_reg(0x15, reg);
        res |= es8374_read_reg(0x1a, &reg);        //disable lout
        reg |= 0x20;
        res |= es8374_write_reg( 0x1a, reg);
        reg &= 0xf7;
        res |= es8374_write_reg( 0x1a, reg);
        res |= es8374_write_reg(0x1D, 0x02);      // mute speaker
        res |= es8374_write_reg(0x1E, 0xa0);      // disable class d

        res |= es8374_set_voice_mute(false);
    }

    return res;
}

error_t es8374_stop(codec_mode_t mode)
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    if (mode == CODEC_MODE_LINE_IN) {
        res |= es8374_read_reg(0x1a, &reg);       //disable lout
        reg |= 0x08;
        res |= es8374_write_reg( 0x1a, reg);
        reg &= 0x9f;
        res |= es8374_write_reg( 0x1a, reg);
        res |= es8374_write_reg(0x1D, 0x12);      // mute speaker
        res |= es8374_write_reg(0x1E, 0x20);      // disable class d
        res |= es8374_read_reg(0x1c, &reg);        // disable spkmixer
        reg &= 0xbf;
        res |= es8374_write_reg( 0x1c, reg);
        res |= es8374_write_reg(0x1F, 0x00);      // spk set
    }
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_set_voice_mute(true);

        res |= es8374_read_reg(0x1a, &reg);        //disable lout
        reg |= 0x08;
        res |= es8374_write_reg( 0x1a, reg);
        reg &= 0xdf;
        res |= es8374_write_reg( 0x1a, reg);
        res |= es8374_write_reg(0x1D, 0x12);      // mute speaker
        res |= es8374_write_reg(0x1E, 0x20);      // disable class d
        res |= es8374_read_reg(0x15, &reg);        //power up dac
        reg |= 0x20;
        res |= es8374_write_reg(0x15, reg);
    }
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {

        res |= es8374_read_reg(0x10, &reg);       //power up adc and input
        reg |= 0xc0;
        res |= es8374_write_reg(0x10, reg);
        res |= es8374_read_reg(0x21, &reg);       //power up adc and input
        reg |= 0xc0;
        res |= es8374_write_reg(0x21, reg);
    }

    return res;
}

error_t es8374_i2s_config_clock(es_i2s_clock_t cfg)
{

    error_t res = RESULT_OK;
    uint8_t reg = 0;

    res |= es8374_read_reg(0x0f, &reg);       //power up adc and input
    reg &= 0xe0;
    int divratio = 0;
    switch (cfg.sclk_div) {
        case MCLK_DIV_1:
            divratio = 1;
            break;
        case MCLK_DIV_2: // = 2,
            divratio = 2;
            break;
        case MCLK_DIV_3: // = 3,
            divratio = 3;
            break;
        case MCLK_DIV_4: // = 4,
            divratio = 4;
            break;
        case MCLK_DIV_5: // = 20,
            divratio = 5;
            break;
        case MCLK_DIV_6: // = 5,
            divratio = 6;
            break;
        case MCLK_DIV_7: //  = 29,
            divratio = 7;
            break;
        case MCLK_DIV_8: // = 6,
            divratio = 8;
            break;
        case MCLK_DIV_9: // = 7,
            divratio = 9;
            break;
        case MCLK_DIV_10: // = 21,
            divratio = 10;
            break;
        case MCLK_DIV_11: // = 8,
            divratio = 11;
            break;
        case MCLK_DIV_12: // = 9,
            divratio = 12;
            break;
        case MCLK_DIV_13: // = 30,
            divratio = 13;
            break;
        case MCLK_DIV_14: // = 31
            divratio = 14;
            break;
        case MCLK_DIV_15: // = 22,
            divratio = 15;
            break;
        case MCLK_DIV_16: // = 10,
            divratio = 16;
            break;
        case MCLK_DIV_17: // = 23,
            divratio = 17;
            break;
        case MCLK_DIV_18: // = 11,
            divratio = 18;
            break;
        case MCLK_DIV_20: // = 24,
            divratio = 19;
            break;
        case MCLK_DIV_22: // = 12,
            divratio = 20;
            break;
        case MCLK_DIV_24: // = 13,
            divratio = 21;
            break;
        case MCLK_DIV_25: // = 25,
            divratio = 22;
            break;
        case MCLK_DIV_30: // = 26,
            divratio = 23;
            break;
        case MCLK_DIV_32: // = 27,
            divratio = 24;
            break;
        case MCLK_DIV_33: // = 14,
            divratio = 25;
            break;
        case MCLK_DIV_34: // = 28,
            divratio = 26;
            break;
        case MCLK_DIV_36: // = 15,
            divratio = 27;
            break;
        case MCLK_DIV_44: // = 16,
            divratio = 28;
            break;
        case MCLK_DIV_48: // = 17,
            divratio = 29;
            break;
        case MCLK_DIV_66: // = 18,
            divratio = 30;
            break;
        case MCLK_DIV_72: // = 19,
            divratio = 31;
            break;
        default:
            break;
    }
    reg |= divratio;
    res |= es8374_write_reg(0x0f, reg);

    int dacratio_l = 0;
    int dacratio_h = 0;

    switch (cfg.lclk_div) {
        case LCLK_DIV_128:
            dacratio_l = 128 % 256;
            dacratio_h = 128 / 256;
            break;
        case LCLK_DIV_192:
            dacratio_l = 192 % 256;
            dacratio_h = 192 / 256;
            break;
        case LCLK_DIV_256:
            dacratio_l = 256 % 256;
            dacratio_h = 256 / 256;
            break;
        case LCLK_DIV_384:
            dacratio_l = 384 % 256;
            dacratio_h = 384 / 256;
            break;
        case LCLK_DIV_512:
            dacratio_l = 512 % 256;
            dacratio_h = 512 / 256;
            break;
        case LCLK_DIV_576:
            dacratio_l = 576 % 256;
            dacratio_h = 576 / 256;
            break;
        case LCLK_DIV_768:
            dacratio_l = 768 % 256;
            dacratio_h = 768 / 256;
            break;
        case LCLK_DIV_1024:
            dacratio_l = 1024 % 256;
            dacratio_h = 1024 / 256;
            break;
        case LCLK_DIV_1152:
            dacratio_l = 1152 % 256;
            dacratio_h = 1152 / 256;
            break;
        case LCLK_DIV_1408:
            dacratio_l = 1408 % 256;
            dacratio_h = 1408 / 256;
            break;
        case LCLK_DIV_1536:
            dacratio_l = 1536 % 256;
            dacratio_h = 1536 / 256;
            break;
        case LCLK_DIV_2112:
            dacratio_l = 2112 % 256;
            dacratio_h = 2112 / 256;
            break;
        case LCLK_DIV_2304:
            dacratio_l = 2304 % 256;
            dacratio_h = 2304 / 256;
            break;
        case LCLK_DIV_125:
            dacratio_l = 125 % 256;
            dacratio_h = 125 / 256;
            break;
        case LCLK_DIV_136:
            dacratio_l = 136 % 256;
            dacratio_h = 136 / 256;
            break;
        case LCLK_DIV_250:
            dacratio_l = 250 % 256;
            dacratio_h = 250 / 256;
            break;
        case LCLK_DIV_272:
            dacratio_l = 272 % 256;
            dacratio_h = 272 / 256;
            break;
        case LCLK_DIV_375:
            dacratio_l = 375 % 256;
            dacratio_h = 375 / 256;
            break;
        case LCLK_DIV_500:
            dacratio_l = 500 % 256;
            dacratio_h = 500 / 256;
            break;
        case LCLK_DIV_544:
            dacratio_l = 544 % 256;
            dacratio_h = 544 / 256;
            break;
        case LCLK_DIV_750:
            dacratio_l = 750 % 256;
            dacratio_h = 750 / 256;
            break;
        case LCLK_DIV_1000:
            dacratio_l = 1000 % 256;
            dacratio_h = 1000 / 256;
            break;
        case LCLK_DIV_1088:
            dacratio_l = 1088 % 256;
            dacratio_h = 1088 / 256;
            break;
        case LCLK_DIV_1496:
            dacratio_l = 1496 % 256;
            dacratio_h = 1496 / 256;
            break;
        case LCLK_DIV_1500:
            dacratio_l = 1500 % 256;
            dacratio_h = 1500 / 256;
            break;
        default:
            break;
    }
    res |= es8374_write_reg( 0x06, dacratio_h);  //ADCFsMode,singel SPEED,RATIO=256
    res |= es8374_write_reg( 0x07, dacratio_l);  //ADCFsMode,singel SPEED,RATIO=256

    return res;
}

error_t es8374_config_output_device()
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    reg = 0x1d;

    res = es8374_write_reg(reg, 0x02);
    res |= es8374_read_reg(0x1c, &reg); // set spk mixer
    reg |= 0x80;
    res |= es8374_write_reg(0x1c, reg);
    res |= es8374_write_reg(0x1D, 0x02); // spk set
    res |= es8374_write_reg(0x1F, 0x00); // spk set
    res |= es8374_write_reg(0x1E, 0xA0); // spk on

    return res;
}



error_t es8374_config_input_device()
{
    error_t res = RESULT_OK;
    uint8_t reg = 0;

    res |= es8374_read_reg(0x21, &reg);
    if (res == 0) {
        reg = (reg & 0xcf) | 0x14;
        res |= es8374_write_reg( 0x21, reg);
    }

    return res;
}

error_t es8374_set_mic_gain(es_mic_gain_t gain)
{
    error_t res = RESULT_OK;

    if (gain > MIC_GAIN_MIN && gain < MIC_GAIN_24DB) {
        int gain_n = 0;
        gain_n = (int)gain / 3;
        res = es8374_write_reg(0x22, gain_n | (gain_n << 4)); //MIC PGA
    } else {
        res = -1;
        AD_LOGI("invalid microphone gain!");
    }

    return res;
}

error_t es8374_codec_set_voice_volume(int volume)
{
    error_t res = RESULT_OK;

    if (volume < 0) {
        volume = 192;
    } else if (volume > 96) {
        volume = 0;
    } else {
        volume = 192 - volume * 2;
    }

    res = es8374_write_reg(0x38, volume);

    return res;
}

error_t es8374_codec_get_voice_volume(int *volume)
{
    error_t res = 0;
    uint8_t reg = 0;

    res = es8374_read_reg(0x38, &reg);

    if (res == RESULT_FAIL) {
        *volume = 0;
    } else {
        *volume = (192 - reg) / 2;
        if (*volume > 96) {
            *volume = 100;
        }
    }

    return res;
}

static int es8374_set_adc_dac_volume(int mode, int volume, int dot)
{
    int res = 0;

    if ( volume < -96 || volume > 0 ) {
        AD_LOGI("Warning: volume < -96! or > 0!");
        if (volume < -96) {
            volume = -96;
        } else {
            volume = 0;
        }
    }
    dot = (dot >= 5 ? 1 : 0);
    volume = (-volume << 1) + dot;
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_write_reg(0x25, volume);
    }
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
        res |= es8374_write_reg(0x38, volume);
    }

    return res;
}

static int es8374_set_d2se_pga(es_d2se_pga_t gain)
{
    int res = 0;
    uint8_t reg = 0;

    if (gain > ES8374_PGA_GAIN_MIN && gain < ES8374_PGA_GAIN_MAX) {
        res = es8374_read_reg(0x21, &reg);
        reg &= 0xfb;
        reg |= gain << 2;
        res = es8374_write_reg(0x21, reg); //MIC PGA
    } else {
        res = 0xff;
        AD_LOGI("invalid microphone gain!");
    }

    return res;
}

static int es8374_init_reg(codec_mode_t ms_mode, i2s_format_t fmt, es_i2s_clock_t cfg)
{
    int res = 0;
    uint8_t reg;

    res |= es8374_write_reg(0x00, 0x3F); //IC Rst start
    res |= es8374_write_reg(0x00, 0x03); //IC Rst stop
    res |= es8374_write_reg(0x01, 0x7F); //IC clk on

    res |= es8374_read_reg(0x0F, &reg);
    reg &= 0x7f;
    reg |=  (ms_mode << 7);
    res |= es8374_write_reg( 0x0f, reg); //CODEC IN I2S SLAVE MODE

    res |= es8374_write_reg(0x6F, 0xA0); //pll set:mode enable
    res |= es8374_write_reg(0x72, 0x41); //pll set:mode set
    res |= es8374_write_reg(0x09, 0x01); //pll set:reset on ,set start
    res |= es8374_write_reg(0x0C, 0x22); //pll set:k
    res |= es8374_write_reg(0x0D, 0x2E); //pll set:k
    res |= es8374_write_reg(0x0E, 0xC6); //pll set:k
    res |= es8374_write_reg(0x0A, 0x3A); //pll set:
    res |= es8374_write_reg(0x0B, 0x07); //pll set:n
    res |= es8374_write_reg(0x09, 0x41); //pll set:reset off ,set stop

    res |= es8374_i2s_config_clock(cfg);

    res |= es8374_write_reg(0x24, 0x08); //adc set
    res |= es8374_write_reg(0x36, 0x00); //dac set
    res |= es8374_write_reg(0x12, 0x30); //timming set
    res |= es8374_write_reg(0x13, 0x20); //timming set

    // TODO
    res |= es8374_config_fmt(CODEC_MODE_ENCODE, fmt);
    res |= es8374_config_fmt(CODEC_MODE_DECODE, fmt);

    res |= es8374_write_reg(0x21, 0x50); //adc set: SEL LIN1 CH+PGAGAIN=0DB
    res |= es8374_write_reg(0x22, 0xFF); //adc set: PGA GAIN=0DB
    res |= es8374_write_reg(0x21, 0x14); //adc set: SEL LIN1 CH+PGAGAIN=18DB
    res |= es8374_write_reg(0x22, 0x55); //pga = +15db
    res |= es8374_write_reg(0x08, 0x21); //set class d divider = 33, to avoid the high frequency tone on laudspeaker
    res |= es8374_write_reg(0x00, 0x80); // IC START

    res |= es8374_set_adc_dac_volume(CODEC_MODE_ENCODE, 0, 0);      // 0db
    res |= es8374_set_adc_dac_volume(CODEC_MODE_DECODE, 0, 0);      // 0db

    res |= es8374_write_reg(0x14, 0x8A); // IC START
    res |= es8374_write_reg(0x15, 0x40); // IC START
    res |= es8374_write_reg(0x1A, 0xA0); // monoout set
    res |= es8374_write_reg(0x1B, 0x19); // monoout set
    res |= es8374_write_reg(0x1C, 0x90); // spk set
    res |= es8374_write_reg(0x1D, 0x01); // spk set
    res |= es8374_write_reg(0x1F, 0x00); // spk set
    res |= es8374_write_reg(0x1E, 0x20); // spk on
    res |= es8374_write_reg(0x28, 0x00); // alc set
    res |= es8374_write_reg(0x25, 0x00); // ADCVOLUME on
    res |= es8374_write_reg(0x38, 0x00); // DACVOLUME on
    res |= es8374_write_reg(0x37, 0x30); // dac set
    res |= es8374_write_reg(0x6D, 0x60); //SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT
    res |= es8374_write_reg(0x71, 0x05); //for automute setting
    res |= es8374_write_reg(0x73, 0x70);

    res |= es8374_config_output_device();  //0x3c Enable DAC and Enable Lout/Rout/1/2
    res |= es8374_config_input_device();  //0x00 LINSEL & RINSEL, LIN1/RIN1 as ADC Input; DSSEL,use one DS Reg11; DSR, LINPUT1-RINPUT1
    res |= es8374_codec_set_voice_volume(0);

    res |= es8374_write_reg(0x37, 0x00); // dac set

    return res;
}

error_t es8374_codec_init(codec_config_t *cfg, codec_mode_t codec_mode, void* i2c, int i2c_port)
{
    i2c_handle = i2c;
    if (i2c_port>0){
        i2c_address_es8374 = i2c_port;
    }
    if (es8374_codec_initialized()) {
        AD_LOGW( "The es8374 codec has already been initialized!");
        return RESULT_FAIL;
    }
    error_t res = RESULT_OK;
    es_i2s_clock_t clkdiv;

    clkdiv.lclk_div = LCLK_DIV_256;
    clkdiv.sclk_div = MCLK_DIV_4;

    // TODO
    res |= es8374_stop(codec_mode);
    res |= es8374_init_reg(cfg->i2s.mode, (BIT_LENGTH_16BITS << 4) | cfg->i2s.fmt, clkdiv);
    res |= es8374_set_mic_gain(MIC_GAIN_15DB);
    res |= es8374_set_d2se_pga(ES8374_PGA_GAIN_EN);
    res |= es8374_config_fmt(codec_mode, cfg->i2s.mode);
    res |= es8374_codec_config_i2s(codec_mode, &(cfg->i2s));
    codec_init_flag = 1;
    return res;
}

error_t es8374_codec_deinit(void)
{
    codec_init_flag = 0;
    return es8374_write_reg(0x00, 0x7F); // IC Reset and STOP
}
error_t es8374_codec_config_i2s(codec_mode_t mode, I2SDefinition *iface)
{
    error_t res = RESULT_OK;
    int tmp = 0;
    res |= es8374_config_fmt(CODEC_MODE_BOTH, iface->fmt);
    if (iface->bits == BIT_LENGTH_16BITS) {
        tmp = BIT_LENGTH_16BITS;
    } else if (iface->bits == BIT_LENGTH_24BITS) {
        tmp = BIT_LENGTH_24BITS;
    } else {
        tmp = BIT_LENGTH_32BITS;
    }
    res |= es8374_set_bits_per_sample(CODEC_MODE_BOTH, tmp);
    return res;
}

error_t es8374_codec_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active)
{
    error_t res = RESULT_OK;
    int es_mode_t = 0;
    switch (mode) {
        case CODEC_MODE_ENCODE:
            es_mode_t  = CODEC_MODE_ENCODE;
            break;
        case CODEC_MODE_LINE_IN:
            es_mode_t  = CODEC_MODE_LINE_IN;
            break;
        case CODEC_MODE_DECODE:
            es_mode_t  = CODEC_MODE_DECODE;
            break;
        case CODEC_MODE_BOTH:
            es_mode_t  = CODEC_MODE_BOTH;
            break;
        default:
            es_mode_t = CODEC_MODE_DECODE;
            AD_LOGW( "Codec mode not support, default is decode mode");
            break;
    }
    if (!ctrl_state_active) {
        res = es8374_stop(es_mode_t);
    } else {
        res = es8374_start(es_mode_t);
        AD_LOGD( "start default is decode mode:%d", es_mode_t);
    }
    return res;
}

