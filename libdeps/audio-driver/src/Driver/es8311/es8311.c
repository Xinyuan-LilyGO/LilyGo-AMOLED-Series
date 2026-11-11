/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
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
#include <assert.h>
#include <math.h>
#include "es8311.h"

#ifndef BIT
#define BIT(nr)             (1 << (nr))
#endif

/* ES8311 address
 * 0x32:CE=1;0x30:CE=0
 * 0x32>>1 = 0x19
 * 0x30>>1 = 0x18
 */
#define ES8311_ADDR         0x18

/*
 * to define the clock soure of MCLK
 */
#define FROM_MCLK_PIN       0
#define FROM_SCLK_PIN       1

/*
 * to define whether to reverse the clock
 */
#define INVERT_MCLK         0 // do not invert
#define INVERT_SCLK         0

#define IS_DMIC             0 // Is it a digital microphone

#define MCLK_DIV_FRE        256

static i2c_bus_handle_t i2c_handle = NULL;
static int i2c_address_es8311 = ES8311_ADDR;

/*
 * Clock coefficient structer
 */
struct coeff_div {
    uint32_t mclk;        /* mclk frequency */
    uint32_t rate;        /* sample rate */
    uint8_t pre_div;      /* the pre divider with range from 1 to 8 */
    uint8_t pre_multi;    /* the pre multiplier with x1, x2, x4 and x8 selection */
    uint8_t adc_div;      /* adcclk divider */
    uint8_t dac_div;      /* dacclk divider */
    uint8_t fs_mode;      /* double speed or single speed, =0, ss, =1, ds */
    uint8_t lrck_h;       /* adclrck divider and daclrck divider */
    uint8_t lrck_l;
    uint8_t bclk_div;     /* sclk divider */
    uint8_t adc_osr;      /* adc osr */
    uint8_t dac_osr;      /* dac osr */
};

/* codec hifi mclk clock divider coefficients */
static const struct coeff_div coeff_div[] = {
    //mclk     rate   pre_div  mult  adc_div dac_div fs_mode lrch  lrcl  bckdiv osr
    /* 8k */
    {12288000, 8000 , 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 8000 , 0x03, 0x02, 0x03, 0x03, 0x00, 0x05, 0xff, 0x18, 0x10, 0x10},
    {16384000, 8000 , 0x08, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 8000 , 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 8000 , 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 8000 , 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 8000 , 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 8000 , 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 11.025k */
    {11289600, 11025, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 11025, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 11025, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 12k */
    {12288000, 12000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 12000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 12000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 16k */
    {12288000, 16000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 16000, 0x03, 0x02, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 16000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 16000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 16000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 16000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 16000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 16000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 16000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 22.05k */
    {11289600, 22050, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 22050, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 24k */
    {12288000, 24000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 24000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 24000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 24000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 32k */
    {12288000, 32000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 32000, 0x03, 0x04, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 32000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 32000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 32000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 32000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
    {1024000 , 32000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 44.1k */
    {11289600, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 44100, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 44100, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 48k */
    {12288000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 48000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 48000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 48000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 64k */
    {12288000, 64000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 64000, 0x03, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {16384000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 64000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 64000, 0x01, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {4096000 , 64000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 64000, 0x01, 0x08, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {2048000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0xbf, 0x03, 0x18, 0x18},
    {1024000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 88.2k */
    {11289600, 88200, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 88200, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 88200, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 88200, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 96k */
    {12288000, 96000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 96000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 96000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
};


#define ES_ASSERT(a, format, b, ...) \
    if ((a) != 0) { \
        AD_LOGE( format, ##__VA_ARGS__); \
        return b;\
    }

static int8_t es8311_mclk_src = 0;

int8_t get_es8311_mclk_src(void) {
 return es8311_mclk_src;
}

static error_t es8311_write_reg(uint8_t reg_addr, uint8_t data)
{
    return i2c_bus_write_bytes(i2c_handle, i2c_address_es8311, &reg_addr, sizeof(reg_addr), &data, sizeof(data));
}

static int es8311_read_reg(uint8_t reg_addr)
{
    uint8_t data;
    i2c_bus_read_bytes(i2c_handle, i2c_address_es8311, &reg_addr, sizeof(reg_addr), &data, sizeof(data));
    return (int)data;
}


/*
* look for the coefficient in coeff_div[] table
*/
static int get_coeff(uint32_t mclk, uint32_t rate)
{
    for (unsigned i = 0; i < (sizeof(coeff_div) / sizeof(coeff_div[0])); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
            return i;
    }
    return -1;
}

/*
* set es8311 dac mute or not
* if mute = 0, dac un-mute
* if mute = 1, dac mute
*/
static void es8311_mute(int mute)
{
    uint8_t regv;
    AD_LOGI( "Enter into es8311_mute(), mute = %d", mute);
    regv = es8311_read_reg(ES8311_DAC_REG31) & 0x9f;
    if (mute) {
        es8311_write_reg(ES8311_SYSTEM_REG12, 0x02);
        es8311_write_reg(ES8311_DAC_REG31, regv | 0x60);
        es8311_write_reg(ES8311_DAC_REG32, 0x00);
        es8311_write_reg(ES8311_DAC_REG37, 0x08);
    } else {
        es8311_write_reg(ES8311_DAC_REG31, regv);
        es8311_write_reg(ES8311_SYSTEM_REG12, 0x00);
    }
}

/*
* set es8311 into suspend mode
*/
static void es8311_suspend(void)
{
    AD_LOGI( "Enter into es8311_suspend()");
    es8311_write_reg(ES8311_DAC_REG32, 0x00);
    es8311_write_reg(ES8311_ADC_REG17, 0x00);
    es8311_write_reg(ES8311_SYSTEM_REG0E, 0xFF);
    es8311_write_reg(ES8311_SYSTEM_REG12, 0x02);
    es8311_write_reg(ES8311_SYSTEM_REG14, 0x00);
    es8311_write_reg(ES8311_SYSTEM_REG0D, 0xFA);
    es8311_write_reg(ES8311_ADC_REG15, 0x00);
    es8311_write_reg(ES8311_DAC_REG37, 0x08);
    es8311_write_reg(ES8311_GP_REG45, 0x01);
}


error_t es8311_codec_init(codec_config_t *codec_cfg, i2c_bus_handle_t handle, int8_t mclk_src, int i2c_address)
{
    es8311_mclk_src = mclk_src;
    uint8_t datmp, regv;
    int coeff;
    error_t ret = RESULT_OK;
    i2c_handle = handle;
    assert(i2c_handle != NULL);
    if (i2c_address > 0){
       i2c_address_es8311 = i2c_address;
    }

    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, 0x30);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG02, 0x00);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG03, 0x10);
    ret |= es8311_write_reg(ES8311_ADC_REG16, 0x24);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG04, 0x10);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG05, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0B, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0C, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG10, 0x1F);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG11, 0x7F);
    ret |= es8311_write_reg(ES8311_RESET_REG00, 0x80);
    /*
     * Set Codec into Master or Slave mode
     */
    regv = es8311_read_reg(ES8311_RESET_REG00);
    /*
     * Set master/slave audio interface
     */
    I2SDefinition *i2s_cfg = &(codec_cfg->i2s);
    switch (i2s_cfg->mode) {
        case MODE_MASTER:    /* MASTER MODE */
            AD_LOGI( "ES8311 in Master mode");
            regv |= 0x40;
            break;
        case MODE_SLAVE:    /* SLAVE MODE */
            AD_LOGI( "ES8311 in Slave mode");
            regv &= 0xBF;
            break;
        default:
            regv &= 0xBF;
    }
    ret |= es8311_write_reg(ES8311_RESET_REG00, regv);
    ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, 0x3F);
    /*
     * Select clock source for internal mclk
     */
    switch (get_es8311_mclk_src()) {
        case FROM_MCLK_PIN:
            AD_LOGI( "ES8311 clock source: MCLK");
            regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
            regv &= 0x7F;
            ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
            break;
        case FROM_SCLK_PIN:
            AD_LOGI( "ES8311 clock source: SCLK");
            regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
            regv |= 0x80;
            ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
            break;
        default:
            AD_LOGI( "ES8311 clock source: MCLK");
            regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
            regv &= 0x7F;
            ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
            break;
    }
    int sample_fre = 0;
    int mclk_fre = 0;
    switch (i2s_cfg->rate) {
        case RATE_8K:
            sample_fre = 8000;
            break;
        case RATE_11K:
            sample_fre = 11025;
            break;
        case RATE_16K:
            sample_fre = 16000;
            break;
        case RATE_22K:
            sample_fre = 22050;
            break;
        case RATE_24K:
            sample_fre = 24000;
            break;
        case RATE_32K:
            sample_fre = 32000;
            break;
        case RATE_44K:
            sample_fre = 44100;
            break;
        case RATE_48K:
            sample_fre = 48000;
            break;
        default:
            AD_LOGE( "Unable to configure sample rate %dHz", sample_fre);
            break;
    }
    mclk_fre = sample_fre * MCLK_DIV_FRE;
    coeff = get_coeff(mclk_fre, sample_fre);
    if (coeff < 0) {
        AD_LOGE( "Unable to configure sample rate %dHz with %dHz MCLK", sample_fre, mclk_fre);
        return RESULT_FAIL;
    }
    /*
     * Set clock parammeters
     */
    if (coeff >= 0) {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG02) & 0x07;
        regv |= (coeff_div[coeff].pre_div - 1) << 5;
        datmp = 0;
        switch (coeff_div[coeff].pre_multi) {
            case 1:
                datmp = 0;
                break;
            case 2:
                datmp = 1;
                break;
            case 4:
                datmp = 2;
                break;
            case 8:
                datmp = 3;
                break;
            default:
                break;
        }

        if (get_es8311_mclk_src() == FROM_SCLK_PIN) {
            datmp = 3;     /* DIG_MCLK = LRCK * 256 = BCLK * 8 */
        }
        regv |= (datmp) << 3;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG02, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG05) & 0x00;
        regv |= (coeff_div[coeff].adc_div - 1) << 4;
        regv |= (coeff_div[coeff].dac_div - 1) << 0;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG05, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG03) & 0x80;
        regv |= coeff_div[coeff].fs_mode << 6;
        regv |= coeff_div[coeff].adc_osr << 0;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG03, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG04) & 0x80;
        regv |= coeff_div[coeff].dac_osr << 0;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG04, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG07) & 0xC0;
        regv |= coeff_div[coeff].lrck_h << 0;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG07, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG08) & 0x00;
        regv |= coeff_div[coeff].lrck_l << 0;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG08, regv);

        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06) & 0xE0;
        if (coeff_div[coeff].bclk_div < 19) {
            regv |= (coeff_div[coeff].bclk_div - 1) << 0;
        } else {
            regv |= (coeff_div[coeff].bclk_div) << 0;
        }
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    }

    /*
     * mclk inverted or not
     */
    if (INVERT_MCLK) {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
        regv |= 0x40;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
    } else {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG01);
        regv &= ~(0x40);
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG01, regv);
    }
    /*
     * sclk inverted or not
     */
    if (INVERT_SCLK) {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06);
        regv |= 0x20;
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    } else {
        regv = es8311_read_reg(ES8311_CLK_MANAGER_REG06);
        regv &= ~(0x20);
        ret |= es8311_write_reg(ES8311_CLK_MANAGER_REG06, regv);
    }

    ret |= es8311_write_reg(ES8311_SYSTEM_REG13, 0x10);
    ret |= es8311_write_reg(ES8311_ADC_REG1B, 0x0A);
    ret |= es8311_write_reg(ES8311_ADC_REG1C, 0x6A);

    //es8311_pa_power(true);
    return RESULT_OK;
}

error_t es8311_codec_deinit()
{
    return RESULT_OK;
}

error_t es8311_config_fmt(I2SDefinition fmt)
{
    error_t ret = RESULT_OK;
    uint8_t adc_iface = 0, dac_iface = 0;
    dac_iface = es8311_read_reg(ES8311_SDPIN_REG09);
    adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A);
    switch (fmt.fmt) {
        case I2S_NORMAL:
            AD_LOGD( "ES8311 in I2S Format");
            dac_iface &= 0xFC;
            adc_iface &= 0xFC;
            break;
        case I2S_LEFT:
        case I2S_RIGHT:
            AD_LOGD( "ES8311 in LJ Format");
            adc_iface &= 0xFC;
            dac_iface &= 0xFC;
            adc_iface |= 0x01;
            dac_iface |= 0x01;
            break;
        case I2S_DSP:
            AD_LOGD( "ES8311 in DSP-A Format");
            adc_iface &= 0xDC;
            dac_iface &= 0xDC;
            adc_iface |= 0x03;
            dac_iface |= 0x03;
            break;
        default:
            dac_iface &= 0xFC;
            adc_iface &= 0xFC;
            break;
    }
    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    return ret;
}

error_t es8311_set_bits_per_sample(sample_bits_t bits)
{
    error_t ret = RESULT_OK;
    uint8_t adc_iface = 0, dac_iface = 0;
    dac_iface = es8311_read_reg(ES8311_SDPIN_REG09);
    adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A);
    switch (bits) {
        case BIT_LENGTH_16BITS:
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;
        case BIT_LENGTH_24BITS:
            break;
        case BIT_LENGTH_32BITS:
            dac_iface |= 0x10;
            adc_iface |= 0x10;
            break;
        default:
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;

    }
    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    return ret;
}

error_t es8311_codec_config_i2s(codec_mode_t mode, I2SDefinition *iface)
{
    int ret = RESULT_OK;
    ret |= es8311_set_bits_per_sample(iface->bits);
    ret |= es8311_config_fmt(*iface);
    return ret;
}

error_t es8311_codec_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active)
{
    error_t ret = RESULT_OK;
    codec_mode_t es_mode = CODEC_MODE_MIN;

    switch (mode) {
        case CODEC_MODE_ENCODE:
            es_mode  = CODEC_MODE_ENCODE;
            break;
        case CODEC_MODE_LINE_IN:
            es_mode  = CODEC_MODE_LINE_IN;
            break;
        case CODEC_MODE_DECODE:
            es_mode  = CODEC_MODE_DECODE;
            break;
        case CODEC_MODE_BOTH:
            es_mode  = CODEC_MODE_BOTH;
            break;
        default:
            es_mode = CODEC_MODE_DECODE;
            AD_LOGW("Codec mode not support, default is decode mode");
            break;
    }

    if (ctrl_state_active) {
        ret |= es8311_start(es_mode);
    } else {
        AD_LOGW("The codec is about to stop");
        ret |= es8311_stop(es_mode);
    }

    return ret;
}


error_t es8311_start(codec_mode_t mode)
{
    error_t ret = RESULT_OK;
    uint8_t adc_iface = 0, dac_iface = 0;

    dac_iface = es8311_read_reg(ES8311_SDPIN_REG09) & 0xBF;
    adc_iface = es8311_read_reg(ES8311_SDPOUT_REG0A) & 0xBF;
    adc_iface |= BIT(6);
    dac_iface |= BIT(6);

    if (mode == CODEC_MODE_LINE_IN) {
        AD_LOGE( "The codec es8311 doesn't support CODEC_MODE_LINE_IN mode");
        return RESULT_FAIL;
    }
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
        adc_iface &= ~(BIT(6));
    }
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
        dac_iface &= ~(BIT(6));
    }

    ret |= es8311_write_reg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311_write_reg(ES8311_SDPOUT_REG0A, adc_iface);

    ret |= es8311_write_reg(ES8311_ADC_REG17, 0xBF);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG0E, 0x02);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG12, 0x00);
    ret |= es8311_write_reg(ES8311_SYSTEM_REG14, 0x1A);

    /*
     * pdm dmic enable or disable
     */
    int regv = 0;
    if (IS_DMIC) {
        regv = es8311_read_reg(ES8311_SYSTEM_REG14);
        regv |= 0x40;
        ret |= es8311_write_reg(ES8311_SYSTEM_REG14, regv);
    } else {
        regv = es8311_read_reg(ES8311_SYSTEM_REG14);
        regv &= ~(0x40);
        ret |= es8311_write_reg(ES8311_SYSTEM_REG14, regv);
    }

    ret |= es8311_write_reg(ES8311_SYSTEM_REG0D, 0x01);
    ret |= es8311_write_reg(ES8311_ADC_REG15, 0x40);
    ret |= es8311_write_reg(ES8311_DAC_REG37, 0x48);
    ret |= es8311_write_reg(ES8311_GP_REG45, 0x00);

    return ret;
}

error_t es8311_stop(codec_mode_t mode)
{
    error_t ret = RESULT_OK;
    es8311_suspend();
    return ret;
}

error_t es8311_codec_set_voice_volume(int volume)
{
    AD_LOGD( "es8311_codec_set_voice_volume:%d", volume);
    if (i2c_handle == 0) return RESULT_FAIL;
    error_t res = RESULT_OK;
    if (volume < 0) {
        volume = 0;
    } else if (volume > 100) {
        volume = 100;
    }
    // Use a logarithmic scale for more natural volume control
    int vol = 0;
    if (volume == 0) {
        vol = 0;
    } else {
        vol = (int)(255.0 * log10(9.0 * volume / 100.0 + 1.0) / log10(10.0));
    }
    es8311_write_reg(ES8311_DAC_REG32, vol);
    return res;
}

error_t es8311_codec_get_voice_volume(int *volume)
{
    if (i2c_handle == 0) return RESULT_FAIL;
    error_t res = RESULT_OK;
    int regv = 0;
    regv = es8311_read_reg(ES8311_DAC_REG32);
    if (regv == RESULT_FAIL) {
        *volume = 0;
        res = RESULT_FAIL;
    } else {
        // Inverse of the logarithmic mapping used in set_voice_volume
        if (regv <= 0) {
            *volume = 0;
        } else {
            double v = (double)regv / 255.0;
            *volume = (int)((pow(10.0, v * log10(10.0)) - 1.0) / 9.0 * 100.0 + 0.5);
            if (*volume > 100) *volume = 100;
        }
    }
    AD_LOGD( "GET: res:%d, volume:%d", regv, *volume);
    return res;
}

error_t es8311_set_voice_mute(bool enable)
{
    AD_LOGD( "Es8311SetVoiceMute volume:%d", enable);
    if (i2c_handle == 0) return RESULT_FAIL;
    es8311_mute(enable);
    return RESULT_OK;
}

error_t es8311_get_voice_mute(int *mute)
{
    if (i2c_handle == 0) return RESULT_FAIL;
    error_t res = RESULT_OK;
    uint8_t reg = 0;
    res = es8311_read_reg(ES8311_DAC_REG31);
    if (res != RESULT_FAIL) {
        reg = (res & 0x20) >> 5;
    }
    *mute = reg;
    return res;
}

error_t es8311_set_mic_gain(es8311_mic_gain_t gain_db)
{
    if (i2c_handle == 0) return RESULT_FAIL;
    error_t res = RESULT_OK;
    res = es8311_write_reg(ES8311_ADC_REG16, gain_db); // MIC gain scale
    return res;
}

void es8311_read_all()
{
    for (int i = 0; i < 0x4A; i++) {
        uint8_t reg = es8311_read_reg(i);
        AD_LOGI("REG:%02x, %02x", reg, i);
    }
}
