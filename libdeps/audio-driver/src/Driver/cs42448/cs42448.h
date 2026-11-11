/*
 * MIT License
 *
 * Copyright (c) 2024 Phil Schatzmann
 *
 * Permission is hereby granted for use on all  products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
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
#pragma once
#include "Driver/DriverConstants.h"
#include "DriverCommon.h"
#include "Platforms/API_I2C.h"

/*
 *  Definitions for Audio registers for CS42448
 */
#define CS42448_Chip_ID 0x01
#define CS42448_Power_Control 0x02
#define CS42448_Functional_Mode 0x03
#define CS42448_Interface_Formats 0x04
#define CS42448_ADC_Control_DAC_DeEmphasis 0x05
#define CS42448_Transition_Control 0x06
#define CS42448_DAC_Channel_Mute 0x07
#define CS42448_AOUT1_Volume_Control 0x08
#define CS42448_AOUT2_Volume_Control 0x09
#define CS42448_AOUT3_Volume_Control 0x0A
#define CS42448_AOUT4_Volume_Control 0x0B
#define CS42448_AOUT5_Volume_Control 0x0C
#define CS42448_AOUT6_Volume_Control 0x0D
#define CS42448_AOUT7_Volume_Control 0x0E
#define CS42448_AOUT8_Volume_Control 0x0F
#define CS42448_DAC_Channel_Invert 0x10
#define CS42448_AIN1_Volume_Control 0x11
#define CS42448_AIN2_Volume_Control 0x12
#define CS42448_AIN3_Volume_Control 0x13
#define CS42448_AIN4_Volume_Control 0x14
#define CS42448_AIN5_Volume_Control 0x15
#define CS42448_AIN6_Volume_Control 0x16
#define CS42448_ADC_Channel_Invert 0x17
#define CS42448_Status_Control 0x18
#define CS42448_Status 0x19
#define CS42448_Status_Mask 0x1A
#define CS42448_MUTEC_Pin_Control 0x1B

/* Bit definitions for Power Control Register */
#define CS42448_PDN (1 << 0)
#define CS42448_PDN_DAC1 (1 << 1)
#define CS42448_PDN_DAC2 (1 << 2)
#define CS42448_PDN_DAC3 (1 << 3)
#define CS42448_PDN_DAC4 (1 << 4)
#define CS42448_PDN_ADC1 (1 << 5)
#define CS42448_PDN_ADC2 (1 << 6)
#define CS42448_PDN_ADC3 (1 << 7)

/* Bit definitions for Functional Mode Register */
#define CS42448_DAC_FM_SLAVE (3 << 6)
#define CS42448_ADC_FM_SLAVE (3 << 4)

/* Bit definitions for Interface Format Register */
#define CS42448_DAC_DIF_I2S (1 << 3)
#define CS42448_ADC_DIF_I2S (1 << 0)
#define CS42448_DAC_DIF_TDM (3 << 4)
#define CS42448_ADC_DIF_TDM (3 << 1)
/**
 * @brief The CS42448 CODEC from Cirrus Logic provides six multi-bit ana-
 * log-to-digital and eight multi-bit digital-to-analog
 * delta-sigma converters.
 */
class CS42448 {
 public:
  enum class Format {
    LeftJustified24Bit = 0,
    I2S = 1,
    RightJustified24Bits = 2,
    RightJustified16Bits = 3,
    OneLine1 = 4,
    OneLine2 = 5,
    TDM = 6
  };
  enum class SoftRamp {
    ImmediateChange = 0,
    ZeroCross,
    SoftRamp,
    SoftRampOnZeroCross
  };
  enum class InterruptControl {
    ActiveHigh = 0,
    ActiveLow,
    OpenDrain,
  };
  enum class FunctionalMode {
    MasterSingleSpeed = 0,
    MasterDoubleSpeed,
    MasterQuadSpeed,
    SlaveAutoDetect
  };
  enum class MasterFrequency {
    MasterSingleSpeed = 0,  //
    MasterDoubleSpeed = 2,
    MasterQuadSpeed = 4,
  };

  // 4.9 Recommended Power-Up Sequence
  //    1. Hold RST low until the power supply and clocks are stable. In this
  //    state,
  //       the control port is reset to its default settings and VQ will remain
  //       low.
  //    2. Bring RST high. The device will initially be in a low power state
  //    with VQ
  //       low. All features will default as described in the "Register Quick
  //       Reference" on page 40.
  //    3. Perform a write operation to the Power Control register ("Power
  //    Control
  //       (Address 02h)" on page 43) to set bit 0 to a '1'b.  This will place
  //       the device in a power down state.
  //    4. Load the desired register settings while keeping the PDN bit set to
  //    '1'b.
  //    5. Mute all DACs. Muting the DACs suppresses any noise associated with
  //    the
  //       CODEC's first initialization after power is applied.
  //    6. Set the PDN bit in the power control register to '0'b. VQ will ramp
  //    to
  //       approximately VA/2 according to the Popguard specification in section
  //       "Popguard" on page 29.
  //    7. Following approximately 2000 LRCK cycles, the device is initialized
  //    and
  //       ready for normal operation.
  //    8. After the CODEC is initialized, wait ~90 LRCK cycles (~1.9 ms @48
  //    kHz) and
  //       then un-mute the DACs.
  //    9. Normal operation begins.
  bool begin(codec_config_t codecCfg, i2c_bus_handle_t handle,
             int i2cAddress = -1) {
    this->codec_config = codecCfg;
    this->i2c = handle;
    if (i2cAddress > 0) {
      this->i2c_address = i2cAddress;
    }

    if (is_active) {
      setMuteADC(true);
      setMuteDAC(true);
    }
    setPowerAll(false);

    // slave mode, MCLK 25.6 MHz max
    if (!setSampleRate(codec_config.i2s.rate)) return false;
    // TDM mode
    if (!setFormat()) return false;
    // single ended ADC
    if (!setADCSingleEnded(true)) return false;
    // soft vol control
    if (!setSoftRampControl(SoftRamp::SoftRampOnZeroCross)) return false;
    // all outputs mute
    setMuteDAC(true);

    // power on requested devices
    is_active = setPowerDevices(true);
    return is_active;
  }

  /// Mute ADC and DAC and then power down
  bool end() {
    setMuteADC(true);
    setMuteDAC(true);
    is_active = false;
    return setPowerDevices(false);
  }

  /**
   * @brief Single-Speed Mode (SSM) supports input sample rates up
   * to 50 kHz and uses a 128x oversampling ratio. Double-Speed Mode (DSM)
   * supports input sample rates up to 100 kHz and uses an oversampling ratio of
   * 64x. Quad-Speed Mode (QSM) supports input sample rates up to 200 kHz and
   * uses an oversampling ratio of 32x (Note: QSM for the ADC is only supported
   * in the I2S, Left-Justified, Right-Justified interface formats.
   */
  bool setSampleRate(int rate) {
    FunctionalMode mode;
    MasterFrequency master_freq;

    switch (codec_config.i2s.mode) {
      case MODE_SLAVE:
        mode = FunctionalMode::SlaveAutoDetect;
        break;

      case MODE_MASTER:
        if (rate > 4000 && rate <= 50000) {
          mode = FunctionalMode::MasterSingleSpeed;
        } else if (rate > 50000 && rate <= 100000) {
          mode = FunctionalMode::MasterDoubleSpeed;
        } else if (rate > 100000 && rate <= 200000) {
          mode = FunctionalMode::MasterQuadSpeed;
        } else {
          return false;
        }
        break;
    }

    if (rate > 4000 && rate <= 50000) {
      master_freq = MasterFrequency::MasterSingleSpeed;
    } else if (rate > 50000 && rate <= 100000) {
      master_freq = MasterFrequency::MasterDoubleSpeed;
    } else if (rate > 100000 && rate <= 200000) {
      master_freq = MasterFrequency::MasterQuadSpeed;
    } else {
      return false;
    }

    uint8_t reg_value = 0;
    reg_value = (uint16_t)mode << 6 | (uint16_t)mode << 4 ||
                (uint16_t)master_freq << 1 | 0;

    if (!writeReg(CS42448_Functional_Mode, reg_value)) return false;

    return true;
  }
  // line starts at 0
  bool setMuteDAC(uint8_t line, bool mute) {
    uint8_t mute_reg_value;
    if (line >= 8) return false;
    if (!readReg(CS42448_DAC_Channel_Mute, &mute_reg_value)) return false;
    setBit(mute_reg_value, line, mute);
    if (!writeReg(CS42448_DAC_Channel_Mute, mute_reg_value)) return false;
    return true;
  }

  bool setMuteDAC(bool mute) {
    uint8_t reg_value = mute ? 0xFF : 0;
    freeze(true);
    if (!writeReg(CS42448_DAC_Channel_Mute, reg_value)) return false;
    freeze(false);
    return true;
  }

  bool setMuteADC(bool mute) {
    uint8_t transition_ctl;
    if (!readReg(CS42448_Transition_Control, &transition_ctl)) return false;
    setBit(transition_ctl, 3, mute);
    if (!writeReg(CS42448_Transition_Control, transition_ctl)) return false;
    return true;
  }

  bool setMute(bool mute) { return setMuteADC(mute) && setMuteDAC(mute); }

  bool setVolumeDAC(uint8_t vol) {
    freeze(true);
    for (uint8_t j = 0; j < 8; j++) {
      setVolumeDAC(j, vol);
    }
    freeze(false);
    return true;
  }

  /// Sets signal levels in 0.5 dB increment from 0 dB to -127.5 dB; Value range
  /// is from 0 to 256; e.g 0 = 0 DB; 180 = -90 DB
  bool setVolumeDAC(uint8_t channel, uint8_t vol) {
    if (channel > 7) return false;
    if (!writeReg(CS42448_AOUT1_Volume_Control + channel, vol)) return false;
    return true;
  }

  int getVolumeDAC(uint8_t channel) {
    uint8_t vol;
    if (channel > 7) return false;
    if (!readReg(CS42448_AOUT1_Volume_Control + channel, &vol)) return -1;
    return vol;
  }

  /// Set volume in 0.5 dB increments. -128 .. 127 is -64dB .. 24dB; 0 = 0dB
  bool setVolumeADC(int8_t volume) {
    freeze(true);
    for (uint8_t j = 0; j < 6; j++) {
      setVolumeADC(j, volume);
    }
    freeze(false);
    return true;
  }

  bool setVolumeADC(uint8_t channel, int8_t volume) {
    if (channel > 5) return false;
    if (!writeReg(CS42448_AIN1_Volume_Control + channel, volume)) return false;
    return true;
  }

  int8_t getVolumeADC(uint8_t channel) {
    int8_t vol;
    if (channel > 5) return false;
    if (!readReg(CS42448_AIN1_Volume_Control + channel, (uint8_t*)&vol))
      return -1;
    return vol;
  }

  /// Power only the selected devices
  bool setPowerDevices(bool active) {
    bool ok_dac = true;
    bool ok_adc = true;
    switch (codec_config.input_device) {
      case ADC_INPUT_NONE:
        break;
      case ADC_INPUT_LINE1:
        ok_dac = setPowerADC(1, active);
        break;
      case ADC_INPUT_LINE2:
        ok_dac = setPowerADC(2, active);
        break;
      case ADC_INPUT_LINE3:
        ok_dac = setPowerADC(3, active);
        break;
      case ADC_INPUT_ALL:
        ok_dac = setPowerADC(active);
        break;
      default:
        ok_dac = false;
        break;
    }
    switch (codec_config.output_device) {
      case DAC_OUTPUT_NONE:
        break;
      case DAC_OUTPUT_LINE1:
        ok_adc = setPowerDAC(1, active);
        break;
      case DAC_OUTPUT_LINE2:
        ok_adc = setPowerDAC(2, active);
        break;
      case DAC_OUTPUT_ALL:
        ok_adc = setPowerDAC(active);
        break;
      default:
        ok_adc = false;
        break;
    }
    return ok_adc && ok_dac;
  }

  /// Power all ADC and DAC and Device
  bool setPowerAll(bool active) {
    return setPowerADC(active) && setPowerDAC(active) && setPower(active);
  }

  /// Device Power
  bool setPower(bool active) {
    uint8_t power;
    if (!readReg(CS42448_Power_Control, &power)) return false;
    setBit(power, 0, active);
    if (!writeReg(CS42448_Power_Control, power)) return false;
    return true;
  }

  /// Power on individual DAC from 1 to 4
  bool setPowerDAC(int dac, bool active) {
    uint8_t power;
    if (dac >= 4) return false;
    if (!readReg(CS42448_Power_Control, &power)) return false;
    setBit(power, dac, active);
    if (!writeReg(CS42448_Power_Control, power)) return false;
    return true;
  }

  bool setPowerDAC(bool active) {
    uint8_t power;
    if (!readReg(CS42448_Power_Control, &power)) return false;
    setBit(power, 1, active);
    setBit(power, 2, active);
    setBit(power, 3, active);
    setBit(power, 4, active);
    if (!writeReg(CS42448_Power_Control, power)) return false;
    return true;
  }

  /// Power on individual ADC from 1 to 3
  bool setPowerADC(int adc, bool active) {
    uint8_t power;
    if (adc > 3) return false;
    if (!readReg(CS42448_Power_Control, &power)) return false;
    setBit(power, adc + 4, active);
    if (!writeReg(CS42448_Power_Control, power)) return false;
    return true;
  }

  bool setPowerADC(bool active) {
    uint8_t power;
    if (!readReg(CS42448_Power_Control, &power)) return false;
    setBit(power, 5, active);
    setBit(power, 6, active);
    setBit(power, 7, active);
    if (!writeReg(CS42448_Power_Control, power)) return false;
    return true;
  }

  /// All mute and volume values are activated on unfreeze
  bool freeze(bool freeze) {
    uint8_t fmt;
    if (!readReg(CS42448_Interface_Formats, &fmt)) return false;
    setBit(fmt, 7, freeze);
    if (!writeReg(CS42448_Interface_Formats, fmt)) return false;
    return true;
  }

  bool setFormat(CS42448::Format fmt) {
    uint8_t fmt_reg_value;
    if (!readReg(CS42448_Interface_Formats, &fmt_reg_value)) return false;
    fmt_reg_value =
        (uint16_t)fmt | ((uint16_t)fmt << 3) | ((uint16_t)fmt_reg_value & 0x80);
    if (!writeReg(CS42448_Interface_Formats, fmt_reg_value)) return false;
    return true;
  }

  bool setSoftRampControl(CS42448::SoftRamp softRamp) {
    uint8_t transition_ctl;
    if (!readReg(CS42448_Transition_Control, &transition_ctl)) return false;
    transition_ctl = (uint16_t)softRamp | ((uint16_t)softRamp << 5) |
                     (transition_ctl & 0x9F);
    if (!writeReg(CS42448_Transition_Control, transition_ctl)) return false;
    return true;
  }

  bool setAutoMute(bool autoMute) {
    uint8_t transition_ctl;
    if (!readReg(CS42448_Transition_Control, &transition_ctl)) return false;
    setBit(transition_ctl, 4, autoMute);
    if (!writeReg(CS42448_Transition_Control, transition_ctl)) return false;
    return true;
  }

  bool setDACInvert(uint8_t channel, int invert) {
    uint8_t invert_reg_value;
    if (!readReg(CS42448_DAC_Channel_Invert, &invert_reg_value)) return false;
    setBit(invert_reg_value, channel, invert);
    if (!writeReg(CS42448_DAC_Channel_Invert, invert_reg_value)) return false;
    return true;
  }

  bool setDACInvert(int invert) {
    uint8_t invert_reg_value = invert ? 0xFF : 0;
    if (!writeReg(CS42448_DAC_Channel_Invert, invert_reg_value)) return false;
    return true;
  }

  bool setADCInvert(uint8_t channel, int invert) {
    uint8_t invert_reg_value;
    if (!readReg(CS42448_ADC_Channel_Invert, &invert_reg_value)) return false;
    setBit(invert_reg_value, channel, invert);
    if (!writeReg(CS42448_ADC_Channel_Invert, invert_reg_value)) return false;
    return true;
  }

  bool setADCInvert(int invert) {
    uint8_t invert_reg_value = invert ? 0xFF : 0;
    if (!writeReg(CS42448_ADC_Channel_Invert, invert_reg_value)) return false;
    return true;
  }

  bool setInterrupControl(InterruptControl value) {
    uint8_t control_reg_value;
    if (!readReg(CS42448_Status_Control, &control_reg_value)) return false;
    control_reg_value = ((uint16_t)value << 2) | (control_reg_value & 0xC);
    if (!writeReg(CS42448_Status_Control, control_reg_value)) return false;
    return true;
  }

  bool getInterruptStatus(uint8_t& status) {
    if (!readReg(CS42448_Status, &status)) return false;
    status &= CS42448_Status_Mask;
    return true;
  }

  bool setADCHighPassFilterFreeze12(bool freeze) {
    uint8_t control_reg_value;
    if (!readReg(CS42448_ADC_Control_DAC_DeEmphasis, &control_reg_value))
      return false;
    setBit(control_reg_value, 7, freeze);
    if (!writeReg(CS42448_ADC_Control_DAC_DeEmphasis, control_reg_value))
      return false;
    return true;
  }

  bool setADCHighPassFilterFreeze3(bool freeze) {
    uint8_t control_reg_value;
    if (!readReg(CS42448_ADC_Control_DAC_DeEmphasis, &control_reg_value))
      return false;
    setBit(control_reg_value, 6, freeze);
    if (!writeReg(CS42448_ADC_Control_DAC_DeEmphasis, control_reg_value))
      return false;
    return true;
  }

  bool setDACDeEmphasisControl(bool on) {
    uint8_t control_reg_value;
    if (!readReg(CS42448_ADC_Control_DAC_DeEmphasis, &control_reg_value))
      return false;
    setBit(control_reg_value, 6, on);
    if (!writeReg(CS42448_ADC_Control_DAC_DeEmphasis, control_reg_value))
      return false;
    return true;
  }

  bool setADCSingleEnded(int adc, bool on) {
    uint8_t control_reg_value;
    if (adc > 3) return false;
    if (adc < 1) return false;
    if (!readReg(CS42448_ADC_Control_DAC_DeEmphasis, &control_reg_value))
      return false;
    setBit(control_reg_value, 5 - adc, on);
    if (!writeReg(CS42448_ADC_Control_DAC_DeEmphasis, control_reg_value))
      return false;
    return true;
  }

  bool setADCSingleEnded(bool on) {
    uint8_t control_reg_value;
    if (!readReg(CS42448_ADC_Control_DAC_DeEmphasis, &control_reg_value))
      return false;
    setBit(control_reg_value, 2, on);
    setBit(control_reg_value, 3, on);
    setBit(control_reg_value, 4, on);
    if (!writeReg(CS42448_ADC_Control_DAC_DeEmphasis, control_reg_value))
      return false;
    return true;
  }

  uint8_t getChipID() {
    uint8_t result = -1;
    readReg(CS42448_Chip_ID, &result);
    return result >> 4;
  }

  uint8_t getChipRevision() {
    uint8_t result = -1;
    readReg(CS42448_Chip_ID, &result);
    return result & 0xF;
  }

 protected:
  codec_config_t codec_config;
  i2c_bus_handle_t i2c;
  int i2c_address = 0x48;
  bool is_active = false;

  /// Set I2S format based on codec_config
  bool setFormat() {
    if (codec_config.i2s.signal_type == SIGNAL_TDM) {
      return setFormat(Format::TDM);
    } else {
      switch (codec_config.i2s.fmt) {
        case I2S_NORMAL:
          return setFormat(Format::I2S);
        case I2S_LEFT:
          return setFormat(Format::LeftJustified24Bit);
        case I2S_RIGHT:
          Format fmt;
          if (codec_config.i2s.bits == BIT_LENGTH_16BITS) {
            fmt = Format::RightJustified16Bits;
          } else if (codec_config.i2s.bits == BIT_LENGTH_16BITS) {
            fmt = Format::RightJustified24Bits;
          } else
            return false;
          return setFormat(fmt);
        default:
          return false;
      }
    }
  }

  bool writeReg(uint8_t reg, uint8_t value) { return writeReg(reg, &value, 1); }

  bool writeReg(uint8_t reg, uint8_t* value, int len) {
    return i2c_bus_write_bytes(i2c, i2c_address, &reg, 1, value, len) ==
           RESULT_OK;
  }

  bool readReg(uint8_t reg, uint8_t* outdata, int datalen = 1) {
    return i2c_bus_read_bytes(i2c, i2c_address, &reg, 1, outdata, datalen) ==
           RESULT_OK;
  }

  inline uint8_t setBit(uint8_t number, uint8_t n, bool x) {
    return (number & ~((uint8_t)1 << n)) | ((uint8_t)x << n);
  }
};
