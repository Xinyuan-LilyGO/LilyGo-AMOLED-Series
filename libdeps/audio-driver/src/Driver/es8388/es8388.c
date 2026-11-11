/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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
#include "es8388.h"

typedef enum {
  ES8388_OUTPUT_MIN = -1,
  ES8388_OUTPUT_LOUT1 = 0x04,
  ES8388_OUTPUT_ROUT1 = 0x08,
  ES8388_OUTPUT_SPK   = 0x09,
  ES8388_OUTPUT_LOUT2 = 0x10,
  ES8388_OUTPUT_ROUT2 = 0x20,
  ES8388_OUTPUT_ALL = 0x3c,
  ES8388_OUTPUT_MAX,
} es_output_device_t;


#include <string.h>
// #include "audio_hal/board_pins_config.h"

static i2c_bus_handle_t i2c_handle = NULL;
static int dac_power = 0x3c;
static int address = ES8388_ADDR;
static int esp8388_volume_hack = AI_THINKER_ES8388_VOLUME_HACK;

#define ES_ASSERT(a, format, b, ...) \
  if ((a) != 0) {                    \
    AD_LOGE(format, ##__VA_ARGS__);  \
    return b;                        \
  }

static error_t es_write_reg(uint8_t slave_addr, uint8_t reg_add, uint8_t data) {
  return i2c_bus_write_bytes(i2c_handle, slave_addr, &reg_add, sizeof(reg_add),
                             &data, sizeof(data));
}

static error_t es_read_reg(uint8_t reg_add, uint8_t *p_data) {
  return i2c_bus_read_bytes(i2c_handle, address, &reg_add, sizeof(reg_add),
                            p_data, 1);
}

void es8388_read_all() {
  AD_TRACED();
  for (int i = 0; i < 50; i++) {
    uint8_t reg = 0;
    es_read_reg(i, &reg);
    AD_LOGI("%x: %x", i, reg);
  }
}

error_t es8388_write_reg(uint8_t reg_add, uint8_t data) {
  return es_write_reg(address, reg_add, data);
}

error_t es8388_read_reg(uint8_t reg_add, uint8_t *p_data) {
  return i2c_bus_read_bytes(i2c_handle, address, &reg_add, sizeof(reg_add),
                            p_data, 1);
}

/**
 * @brief Configure ES8388 ADC and DAC volume. Basicly you can consider this as
 * ADC and DAC gain
 *
 * @param mode:             set ADC or DAC or all
 * @param volume:           -96 ~ 0              for example
 * Es8388SetAdcDacVolume(CODEC_MODE_ENCODE, 30, 6); means set ADC volume -30.5db
 * @param dot:              whether include 0.5. for example
 * Es8388SetAdcDacVolume(CODEC_MODE_ENCODE, 30, 4); means set ADC volume -30db
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
static int es8388_set_adc_dac_volume(int mode, int volume, int dot) {
  AD_LOGD("es8388_set_adc_dac_volume: %d.%d", volume, dot);
  int res = 0;
  if (volume < -96 || volume > 0) {
    AD_LOGW("Warning: volume < -96! or > 0!");
    if (volume < -96)
      volume = -96;
    else
      volume = 0;
  }
  dot = (dot >= 5 ? 1 : 0);
  volume = (-volume << 1) + dot;
  if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
    res |= es_write_reg(address, ES8388_ADCCONTROL8, volume);
    res |= es_write_reg(address, ES8388_ADCCONTROL9,
                        volume);  // ADC Right Volume=0db
  }
  if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
    res |= es_write_reg(address, ES8388_DACCONTROL5, volume);
    res |= es_write_reg(address, ES8388_DACCONTROL4, volume);
  }
  return res;
}

/**
 * @brief Power Management
 *
 * @param mod:      if ES_POWER_CHIP, the whole chip including ADC and DAC is
 * enabled
 * @param enable:   false to disable true to enable
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
error_t es8388_start(codec_mode_t mode) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t prev_data = 0, data = 0;
  es_read_reg(ES8388_DACCONTROL21, &prev_data);
  if (mode == CODEC_MODE_LINE_IN) {
    res |= es_write_reg(
        address, ES8388_DACCONTROL16,
        0x09);  // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 by pass enable
    res |=
        es_write_reg(address, ES8388_DACCONTROL17,
                     0x50);  // left DAC to left mixer enable  and  LIN signal
                             // to left mixer enable 0db  : bupass enable
    res |=
        es_write_reg(address, ES8388_DACCONTROL20,
                     0x50);  // right DAC to right mixer enable  and  LIN signal
                             // to right mixer enable 0db : bupass enable
    res |= es_write_reg(address, ES8388_DACCONTROL21, 0xC0);  // enable adc
  } else {
    res |= es_write_reg(address, ES8388_DACCONTROL21, 0x80);  // enable dac
  }
  es_read_reg(ES8388_DACCONTROL21, &data);
  if (prev_data != data) {
    res |= es_write_reg(address, ES8388_CHIPPOWER,
                        0xF0);  // start state machine
    // res |= es_write_reg(address, ES8388_CONTROL1, 0x16);
    // res |= es_write_reg(address, ES8388_CONTROL2, 0x50);
    res |= es_write_reg(address, ES8388_CHIPPOWER,
                        0x00);  // start state machine
  }
  if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH ||
      mode == CODEC_MODE_LINE_IN) {
    res |= es_write_reg(address, ES8388_ADCPOWER,
                        0x00);  // power up adc and line in
    AD_LOGD("es8388_start default is mode:%d", mode);
  }
  if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH ||
      mode == CODEC_MODE_LINE_IN) {
    res |= es_write_reg(address, ES8388_DACPOWER,
                        dac_power);  // power up dac and line out
    res |= es8388_set_voice_mute(false);
    AD_LOGD("es8388_start default is mode:%d", mode);
  }

  return res;
}

/**
 * @brief Power Management
 *
 * @param mod:      if ES_POWER_CHIP, the whole chip including ADC and DAC is
 * enabled
 * @param enable:   false to disable true to enable
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
error_t es8388_stop(codec_mode_t mode) {
  AD_TRACED();
  error_t res = RESULT_OK;
  if (mode == CODEC_MODE_LINE_IN) {
    res |= es_write_reg(address, ES8388_DACCONTROL21, 0x80);  // enable dac
    res |= es_write_reg(address, ES8388_DACCONTROL16,
                        0x00);  // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2
    res |= es_write_reg(address, ES8388_DACCONTROL17,
                        0x90);  // only left DAC to left mixer enable 0db
    res |= es_write_reg(address, ES8388_DACCONTROL20,
                        0x90);  // only right DAC to right mixer enable 0db
    return res;
  }
  if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
    res |= es_write_reg(address, ES8388_DACPOWER, 0x00);
    res |= es8388_set_voice_mute(true);
    // res |= Es8388SetAdcDacVolume(CODEC_MODE_DECODE, -96, 5);      // 0db
    // res |= es_write_reg(address, ES8388_DACPOWER, 0xC0);  //power down
    // dac and line out
  }
  if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
    // res |= Es8388SetAdcDacVolume(CODEC_MODE_ENCODE, -96, 5);      // 0db
    res |= es_write_reg(address, ES8388_ADCPOWER,
                        0xFF);  // power down adc and line in
  }
  if (mode == CODEC_MODE_BOTH) {
    res |= es_write_reg(address, ES8388_DACCONTROL21, 0x9C);  // disable
                                                                  // mclk
    //        res |= es_write_reg(address, ES8388_CONTROL1, 0x00);
    //        res |= es_write_reg(address, ES8388_CONTROL2, 0x58);
    //        res |= es_write_reg(address, ES8388_CHIPPOWER, 0xF3);  //stop
    //        state machine
  }

  return res;
}

/**
 * @brief Config I2s clock in MASTER mode
 *
 * @param cfg.sclkDiv:      generate SCLK by dividing MCLK in MSATER mode
 * @param cfg.lclkDiv:      generate LCLK by dividing MCLK in MSATER mode
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
error_t es8388_i2s_config_clock(es_i2s_clock_t cfg) {
  AD_TRACED();
  error_t res = RESULT_OK;
  res |= es_write_reg(address, ES8388_MASTERMODE, cfg.sclk_div);
  res |= es_write_reg(address, ES8388_ADCCONTROL5,
                      cfg.lclk_div);  // ADCFsMode,singel SPEED,RATIO=256
  res |= es_write_reg(address, ES8388_DACCONTROL2,
                      cfg.lclk_div);  // ADCFsMode,singel SPEED,RATIO=256
  return res;
}

error_t es8388_deinit(void) {
  AD_TRACED();
  int res = 0;
  res = es_write_reg(address, ES8388_CHIPPOWER,
                     0xFF);  // reset and stop es8388
#ifdef CONFIG_ESP_LYRAT_V4_3_BOARD
  headphone_detect_deinit();
#endif

  return res;
}

/**
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */
error_t es8388_init(codec_config_t *cfg, i2c_bus_handle_t handle, int addr, int volumeHack) {
  AD_TRACED();
  i2c_handle = handle;
  esp8388_volume_hack = volumeHack;
  // set address
  if (addr>0) address = addr;

  int res = 0;

  res |= es_write_reg(address, ES8388_DACCONTROL3,
                      0x04);  // 0x04 mute/0x00 unmute&ramp;DAC unmute and
                              // disabled digital volume control soft ramp
  /* Chip Control and Power Management */
  res |= es_write_reg(address, ES8388_CONTROL2, 0x50);
  res |= es_write_reg(address, ES8388_CHIPPOWER,
                      0x00);  // normal all and power up all

  // Disable the internal DLL to improve 8K sample rate
  res |= es_write_reg(address, 0x35, 0xA0);
  res |= es_write_reg(address, 0x37, 0xD0);
  res |= es_write_reg(address, 0x39, 0xD0);

  res |= es_write_reg(address, ES8388_MASTERMODE,
                      cfg->i2s.mode);  // CODEC IN I2S SLAVE MODE

  /* dac */
  res |= es_write_reg(address, ES8388_DACPOWER,
                      0xC0);  // disable DAC and disable Lout/Rout/1/2
  res |= es_write_reg(address, ES8388_CONTROL1,
                      0x12);  // Enfr=0,Play&Record Mode,(0x17-both of mic&paly)
  //    res |= es_write_reg(address, ES8388_CONTROL2, 0);
  //    //LPVrefBuf=0,Pdn_ana=0
  res |= es_write_reg(address, ES8388_DACCONTROL1,
                      0x18);  // 1a 0x18:16bit iis , 0x00:24
  res |= es_write_reg(address, ES8388_DACCONTROL2,
                      0x02);  // DACFsMode,SINGLE SPEED; DACFsRatio,256
  res |= es_write_reg(address, ES8388_DACCONTROL16,
                      0x00);  // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2
  res |= es_write_reg(address, ES8388_DACCONTROL17,
                      0x90);  // only left DAC to left mixer enable 0db
  res |= es_write_reg(address, ES8388_DACCONTROL20,
                      0x90);  // only right DAC to right mixer enable 0db
  res |= es_write_reg(address, ES8388_DACCONTROL21,
                      0x80);  // set internal ADC and DAC use the same LRCK
                              // clock, ADC LRCK as internal LRCK
  res |= es_write_reg(address, ES8388_DACCONTROL23, 0x00);  // vroi=0
  res |= es8388_set_adc_dac_volume(CODEC_MODE_DECODE, 0, 0);    // 0db
  dac_power = 0;
  AD_LOGI("output_device: %d", cfg->output_device);
  if (DAC_OUTPUT_LINE1 == cfg->output_device) {
    dac_power = ES8388_OUTPUT_LOUT1 | ES8388_OUTPUT_ROUT1;
  } else if (DAC_OUTPUT_LINE2 == cfg->output_device) {
    dac_power = ES8388_OUTPUT_LOUT2 | ES8388_OUTPUT_ROUT2;
  } else if (DAC_OUTPUT_ALL){
    dac_power = ES8388_OUTPUT_LOUT1 | ES8388_OUTPUT_LOUT2 |
                ES8388_OUTPUT_ROUT1 | ES8388_OUTPUT_ROUT2;
  }
  res |= es_write_reg(address, ES8388_DACPOWER,
                      dac_power);  // 0x3c Enable DAC and Enable Lout/Rout/1/2
  /* adc */
  res |= es_write_reg(address, ES8388_ADCPOWER, 0xFF);

  // // AudioDriver: WORKAROUND_MIC_LINEIN_MIXED
  es_mic_gain_t mic_gain = (es_mic_gain_t)ES8388_DEFAULT_INPUT_GAIN;
  // TODO
  // if (WORKAROUND_MIC_LINEIN_MIXED ){
  //     // logic for Audiokit: line 1 does not work, so we also use line2 but
  //     with a different gain if (ADC_INPUT_LINE1 == cfg->input_device){
  //         mic_gain = (es_mic_gain_t)WORKAROUND_ES8388_LINE1_GAIN;
  //         cfg->input_device= ADC_INPUT_LINE2;
  //     }
  // }
  // res |= es_write_reg(address, ES8388_ADCCONTROL1, 0xbb); // MIC Left and
  // Right channel PGA gain res |= es_write_reg(address, ES8388_ADCCONTROL1,
  // ES8388_DEFAULT_INPUT_GAIN); // MIC Left and Right channel PGA gain
  res |= es8388_set_mic_gain(mic_gain);
  int tmp = 0;
  if (ADC_INPUT_LINE1 == cfg->input_device) {
    tmp = ESP8388_INPUT_LINPUT1_RINPUT1;
  } else if (ADC_INPUT_LINE2 == cfg->input_device) {
    tmp = ESP8388_INPUT_LINPUT2_RINPUT2;
  } else {
    tmp = ESP8388_INPUT_DIFFERENCE;
  }
  res |= es_write_reg(address, ES8388_ADCCONTROL2,
                      tmp);  // 0x00 LINSEL & RINSEL, LIN1/RIN1 as ADC Input;
                             // DSSEL,use one DS Reg11; DSR, LINPUT1-RINPUT1
  res |= es_write_reg(address, ES8388_ADCCONTROL3, 0x02);
  res |= es_write_reg(address, ES8388_ADCCONTROL4,
                      0x0d);  // Left/Right data, Left/Right justified mode,
                              // Bits length, I2S format
  res |= es_write_reg(address, ES8388_ADCCONTROL5,
                      0x02);  // ADCFsMode,singel SPEED,RATIO=256
  // ALC for Microphone
  res |= es8388_set_adc_dac_volume(CODEC_MODE_ENCODE, 0, 0);  // 0db
  res |= es_write_reg(address, ES8388_ADCPOWER,
                      0x09);  // Power on ADC, Enable LIN&RIN, Power off
                              // MICBIAS, set int1lp to low power mode
  // es8388_pa_power(cfg->_DAC_OUTPUT!=ES8388_OUTPUT_LINE2);
  //  AD_LOGI("init,out:%02x, in:%02x", cfg->_DAC_OUTPUT, cfg->input_device);
  return res;
}

/**
 * @brief Configure ES8388 I2S format
 *
 * @param mode:           set ADC or DAC or all
 * @param bitPerSample:   see Es8388I2sFmt
 *
 * @return
 *     - (-1) Error
 *     - (0)  Success
 */
error_t es8388_config_fmt(codec_mode_t mode, i2s_format_t fmt) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t reg = 0;
  if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
    res = es_read_reg(ES8388_ADCCONTROL4, &reg);
    reg = reg & 0xfc;
    res |= es_write_reg(address, ES8388_ADCCONTROL4, reg | fmt);
  }
  if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
    res = es_read_reg(ES8388_DACCONTROL1, &reg);
    reg = reg & 0xf9;
    res |= es_write_reg(address, ES8388_DACCONTROL1, reg | (fmt << 1));
  }
  return res;
}

error_t es8388_set_voice_volume_0(int volume) {
  AD_LOGD("es8388_set_voice_volume: %d", volume);
  error_t res = RESULT_OK;
  if (volume < 0)
    volume = 0;
  else if (volume > 100)
    volume = 100;
  volume /= 3;
  // ROUT1VOL LOUT1VOL 0 -> -45dB; 33 -> – 4.5dB
  res = es_write_reg(address, ES8388_DACCONTROL24, volume);
  res |= es_write_reg(address, ES8388_DACCONTROL25, volume);
  // DAC LDACVOL RDACVOL default 0 = 0DB; Default value 192 = – -96 dB
  res |= es_write_reg(address, ES8388_DACCONTROL26, 0);
  res |= es_write_reg(address, ES8388_DACCONTROL27, 0);
  return res;
}


error_t es8388_set_voice_volume_1(int volume) {
  AD_LOGD("es8388_set_voice_volume (HACK 1): %d", volume);
  error_t res = RESULT_OK;
  if (volume < 0)
    volume = 0;
  else if (volume > 100)
    volume = 100;
  volume /= 3;
  // DAC LDACVOL RDACVOL default 0 = 0DB; Default value 192 = – -96 dB
  res = es_write_reg(address, ES8388_DACCONTROL4, 0);
  res |= es_write_reg(address, ES8388_DACCONTROL5, 0);
  // LOUT1 LOUT1 volume: dataheet says only 6 bits
  res |= es_write_reg(address, ES8388_DACCONTROL24, volume);
  res |= es_write_reg(address, ES8388_DACCONTROL25, volume);
  // LOUT2 ROUT2 volume: datasheet says only 6 bits
  res |= es_write_reg(address, ES8388_DACCONTROL26, volume);
  res |= es_write_reg(address, ES8388_DACCONTROL27, volume);
  return res;
}


error_t es8388_set_voice_volume_2(int volume) {
  AD_LOGD("es8388_set_voice_volume (HACK 2): %d", volume);
  error_t res = RESULT_OK;
  if (volume < 0)
    volume = 0;
  else if (volume > 100)
    volume = 100;
  volume = (volume * 63) / 100;
  // LOUT1 RLOUT1 volume: dataheet says only 6 bits
  res = es_write_reg(address, ES8388_DACCONTROL24, volume);
  res |= es_write_reg(address, ES8388_DACCONTROL25, volume);
  // DAC LDACVOL RDACVOL default 0 = 0DB; Default value 192 = – -96 dB
  res |= es_write_reg(address, ES8388_DACCONTROL26, 0);
  res |= es_write_reg(address, ES8388_DACCONTROL27, 0);
  // 30-bit a coefficient for shelving filter
  res |= es_write_reg(address, ES8388_DACCONTROL8, 192 >> 2);
  res |= es_write_reg(address, ES8388_DACCONTROL9, 192 >> 2);
  return res;
}

/**
 * @brief Sets the output volume
 * @param volume: 0 ~ 100
 *
 * @return
 *     - (-1)  Error
 *     - (0)   Success
 */

 error_t es8388_set_voice_volume(int volume) {
  switch(esp8388_volume_hack) {
    case 0:
      return es8388_set_voice_volume_0(volume);
    case 1:
      return es8388_set_voice_volume_1(volume);
    case 2:
      return es8388_set_voice_volume_2(volume);
    default:
      return es8388_set_voice_volume_0(volume);
  }
 }


/**
 * Provides the volume
 * @return
 *           volume
 */
error_t es8388_get_voice_volume(int *volume) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t reg = 0;
  res = es_read_reg(ES8388_DACCONTROL24, &reg);
  if (res == RESULT_FAIL) {
    *volume = 0;
  } else {
    *volume = reg;
    *volume *= 3;
    if (*volume == 99) *volume = 100;
  }
  return res;
}

/**
 * @brief Configure ES8388 data sample bits
 *
 * @param mode:             set ADC or DAC or all
 * @param bitPerSample:   see BitsLength
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
error_t es8388_set_bits_per_sample(codec_mode_t mode,
                                   sample_bits_t bits_length) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t reg = 0;
  int bits = (int)bits_length;

  if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) {
    res = es_read_reg(ES8388_ADCCONTROL4, &reg);
    reg = reg & 0xe3;
    res |= es_write_reg(address, ES8388_ADCCONTROL4, reg | (bits << 2));
  }
  if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) {
    res = es_read_reg(ES8388_DACCONTROL1, &reg);
    reg = reg & 0xc7;
    res |= es_write_reg(address, ES8388_DACCONTROL1, reg | (bits << 3));
  }
  return res;
}

/**
 * @brief Configure ES8388 DAC mute or not. Basically you can use this function
 * to mute the output or unmute
 *
 * @param enable: enable or disable
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
error_t es8388_set_voice_mute(bool enable) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t reg = 0;
  res = es_read_reg(ES8388_DACCONTROL3, &reg);
  reg = reg & 0xC3; // keep 11000011
  int value = enable ? 0x3C : 0; // enable is 00111100 / disable is 00000000
  res |= es_write_reg(address, ES8388_DACCONTROL3, value | reg);
  return res;
}

error_t es8388_get_voice_mute(void) {
  AD_TRACED();
  error_t res = RESULT_OK;
  uint8_t reg = 0;
  res = es_read_reg(ES8388_DACCONTROL3, &reg);
  if (res == RESULT_OK) {
    reg = (reg & 0x04) >> 2;
  }
  return res == RESULT_OK ? reg : res;
}

/**
 * @param gain: Config DAC Output
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
error_t es8388_config_output_device(output_device_t output_device) {
  AD_TRACED();
  AD_LOGI("output_device: %d", output_device);

  uint8_t reg = 0;
  error_t res = es_read_reg(ES8388_DACPOWER, &reg);
  reg = reg & 0xC3; // keep 11000011

  uint8_t value = 0;
  switch (output_device) {
    case DAC_OUTPUT_LINE1:
      value = ES8388_OUTPUT_LOUT1 | ES8388_OUTPUT_ROUT1;
      AD_LOGI("DAC_OUTPUT_LINE2:  0x%x", reg | value);
      break;
    case DAC_OUTPUT_LINE2:
      value = ES8388_OUTPUT_LOUT2 | ES8388_OUTPUT_ROUT2;
      AD_LOGI("DAC_OUTPUT_LINE1: 0x%x",reg | value);
      break;
    case DAC_OUTPUT_ALL:
      value = ES8388_OUTPUT_LOUT1 | ES8388_OUTPUT_LOUT2 |
                  ES8388_OUTPUT_ROUT1 | ES8388_OUTPUT_ROUT2;
      AD_LOGI("DAC_OUTPUT_ALL: 0x%x",reg | value);
      break;
    case DAC_OUTPUT_NONE:
      value = 0;
      AD_LOGI("DAC_OUTPUT_NONE: 0x%x",reg | value);
      break;
  }
  res |= es_write_reg(address, ES8388_DACPOWER, reg | value);
  return res;
}

/**
 * @param gain: Config ADC input
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
error_t es8388_config_input_device(es8388_input_device_t input) {
  AD_TRACED();
  error_t res;
  uint8_t reg = 0;
  res = es_read_reg(ES8388_ADCCONTROL2, &reg);
  reg = reg & 0x0f;
  res |= es_write_reg(address, ES8388_ADCCONTROL2, reg | input);
  return res;
}

/**
 * @param gain: see es_mic_gain_t
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
error_t es8388_set_mic_gain(es_mic_gain_t gain) {
  AD_LOGD("es8388_set_mic_gain: %d", gain);
  error_t res, gain_n;
  gain_n = (int)gain / 3;
  gain_n = (gain_n << 4) + gain_n;
  res = es_write_reg(address, ES8388_ADCCONTROL1, gain_n);  // MIC PGA
  return res;
}

int es8388_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active) {
  AD_LOGD("es8388_ctrl_state_active: %d, %d", mode, ctrl_state_active);
  int res = 0;
  int es_mode_t = 0;
  switch (mode) {
    case CODEC_MODE_ENCODE:
      es_mode_t = CODEC_MODE_ENCODE;
      break;
    case CODEC_MODE_LINE_IN:
      es_mode_t = CODEC_MODE_LINE_IN;
      break;
    case CODEC_MODE_DECODE:
      es_mode_t = CODEC_MODE_DECODE;
      break;
    case CODEC_MODE_BOTH:
      es_mode_t = CODEC_MODE_BOTH;
      break;
    default:
      es_mode_t = CODEC_MODE_DECODE;
      AD_LOGW("Codec mode not support, default is decode mode");
      break;
  }
  if (!ctrl_state_active) {
    res = es8388_stop(es_mode_t);
  } else {
    res = es8388_start(es_mode_t);
    AD_LOGD("start default is decode mode:%d", es_mode_t);
  }
  return res;
}

error_t es8388_config_i2s(codec_mode_t mode, I2SDefinition *iface) {
  AD_TRACED();
  error_t res = RESULT_OK;
  int tmp = 0;
  res |= es8388_config_fmt(CODEC_MODE_BOTH, iface->fmt);
  if (iface->bits == BIT_LENGTH_16BITS) {
    tmp = BIT_LENGTH_16BITS;
  } else if (iface->bits == BIT_LENGTH_24BITS) {
    tmp = BIT_LENGTH_24BITS;
  } else {
    tmp = BIT_LENGTH_32BITS;
  }
  res |= es8388_set_bits_per_sample(CODEC_MODE_BOTH, tmp);
  return res;
}
