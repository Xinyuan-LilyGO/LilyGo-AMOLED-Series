#include "PCBCUPID_NAU8325.h"
//#include "esp_log.h"

#define MASTER_CLK_MIN 2048000
#define MASTER_CLK_MAX 49152000

// N1 divider options
const SrcAttr PCBCUPID_NAU8325::mclk_n1_div[] = {
    {1, 0x0},
    {2, 0x1},
    {3, 0x2}};

// N2 divider options
const SrcAttr PCBCUPID_NAU8325::mclk_n2_div[] = {
    {0, 0x0},
    {1, 0x1},
    {2, 0x2},
    {3, 0x3},
    {4, 0x4}};

// N3 multiplier options
const SrcAttr PCBCUPID_NAU8325::mclk_n3_mult[] = {
    {0, 0x1},
    {1, 0x2},
    {2, 0x3},
    {3, 0x4}};

// DAC oversampling options
const OsrAttr PCBCUPID_NAU8325::osr_dac_sel[] = {
    {64, 2},  // OSR 64, SRC 1/4
    {256, 0}, // OSR 256, SRC 1
    {128, 1}, // OSR 128, SRC 1/2
    {0, 0},
    {32, 3} // OSR 32, SRC 1/8
};

// accessing the table as expected based oon sample rate and mclk
const SRateAttr PCBCUPID_NAU8325::target_srate_table[] = {
    {48000, 2, true, {12288000, 19200000, 24000000}},
    {16000, 1, false, {4096000, 6400000, 8000000}},
    {8000, 0, false, {2048000, 3200000, 4000000}},
    {44100, 2, true, {11289600, 17640000, 22050000}},
    {64000, 3, false, {16384000, 25600000, 32000000}},
    {96000, 3, true, {24576000, 38400000, 48000000}},
    {12000, 0, true, {3072000, 4800000, 6000000}},
    {24000, 1, true, {6144000, 9600000, 12000000}},
    {32000, 2, false, {8192000, 12800000, 16000000}}};

uint16_t set_ratio = 0; // Clock ratio for enabling the I2S device

bool alc_enable = true;
bool clock_detection = true;
bool clock_det_data = true;
uint32_t dac_vref_microvolt = 2880000;
uint32_t vref_impedance_ohms = 125000;

PCBCUPID_NAU8325::PCBCUPID_NAU8325(TwoWire &wire)
    : i2c(wire), i2c_addr(NAU8325_I2C_ADDR) {}

bool PCBCUPID_NAU8325::begin(uint32_t fs, uint8_t bits_per_sample, uint16_t ratio)
{

  log_i("[NAU8325] beginDynamic() starting...");
  log_i("  fs = %lu Hz", fs);
  log_i("  ratio = %u Hz", ratio);
  log_i("  bits = %u", bits_per_sample);

  if (bits_per_sample != 16 && bits_per_sample != 24 && bits_per_sample != 32)
  {

    log_i("[NAU8325] Unsupported bit width: %u\n", bits_per_sample);
    return false;
  }

  set_ratio = ratio;
  uint32_t mclk = fs * ratio;
  resetChip();
  delay(5);

  if (!setSysClock(mclk))
    return false;

  if (!configureAudio(fs, mclk, bits_per_sample))
    return false;

  if (!setI2SFormat(I2S_STD, NORMAL_BCLK))
    return false;

  // ***********Analog + ALC + Vref Config ***********
  initRegisters();

  // Wait for analog to settle
  delay(50);

  // Enable Analog/Output Blocks (DAPM Equivalent)
  enableDAPMBlocks();

  // Set default volume AFTER analog path is ready
  setVolume(0xFD, 0xFD);

  softMute(false);

  /*default powerOn*/
  powerOn();

  log_i("[NAU8325] beginDynamic() complete - sound should play");
  return true;
}

/*If the user didn't provide the ratio it will call this function with default ratio*/
bool PCBCUPID_NAU8325::begin(uint32_t fs, uint8_t bits)
{
  return begin(fs, bits, 256); // Passing 256 as the default ratio for clock division
}

/*if the user didn't pass any of the parameter like fs,bits,ratio*/
bool PCBCUPID_NAU8325::begin()
{
  return begin(44100, 16, 256); // FS - 44100, BitRate - 16, Ratio 256
}

void PCBCUPID_NAU8325::enableDAPMBlocks()
{
  // Enable DAC L/R in ENA_CTRL (R04)
  writeRegisterBits(NAU8325_R04_ENA_CTRL,
                    NAU8325_ENA_CTRL_DAC_EN_MASK,
                    NAU8325_ENA_CTRL_DAC_EN_MASK);

  // Enable analog power-up blocks (e.g., VMID, Class-D) in ANALOG_CONTROL_1 (R61)
  writeRegisterBits(NAU8325_R61_ANALOG_CONTROL_1,
                    NAU8325_ANALOG_CTRL1_EN_MASK,
                    NAU8325_ANALOG_CTRL1_EN_MASK);

  // Enable speaker path routing (ANALOG_CONTROL_6 R66)
  writeRegisterBits(NAU8325_R66_ANALOG_CONTROL_6,
                    NAU8325_ANALOG_CTRL6_EN_MASK,
                    NAU8325_ANALOG_CTRL6_EN_MASK);
}

void PCBCUPID_NAU8325::resetChip()
{
  writeRegister(0x0000, 0x0001);
  delay(2);
  writeRegister(0x0000, 0x0000);
  delay(2);
}

uint16_t PCBCUPID_NAU8325::readDeviceId()
{
  uint16_t id;
  if (readRegister(NAU8325_R02_DEVICE_ID, id))
  {
    return id;
  }
  else
  {
    return 0xFFFF; // Invalid or failed read
  }
}

bool PCBCUPID_NAU8325::applySampleRateClocks(const SRateAttr *srate, int n1_sel, int mclk_mult_sel, int n2_sel)
{
  if (!srate || n1_sel < 0 || n1_sel >= 3 || n2_sel < 0 || n2_sel >= 5)
  {
    log_i("[NAU8325] Invalid N1/N2 divider index.");
    return false;
  }

  // --- Debug ---
  log_i("--- Debugging clock dividers ---");
  log_i("n1_sel=%d (val=%d)", n1_sel, mclk_n1_div[n1_sel].val);
  log_i("n2_sel=%d (val=%d)", n2_sel, mclk_n2_div[n2_sel].val);
  log_i("mult_sel=%d", mclk_mult_sel);

  // Set sample rate range and max mode
  writeRegisterBits(NAU8325_R40_CLK_DET_CTRL,
                    NAU8325_REG_SRATE_MASK | NAU8325_REG_DIV_MAX,
                    (srate->range << NAU8325_REG_SRATE_SFT) | (srate->max ? NAU8325_REG_DIV_MAX : 0));
  // Batch update CLK_CTRL (0x03)
  uint16_t clk_ctrl_val = 0;
  clk_ctrl_val |= mclk_n2_div[n2_sel].val;                            // MCLK_SRC
  clk_ctrl_val |= mclk_n1_div[n1_sel].val << NAU8325_CLK_MUL_SRC_SFT; // N1
  if (mclk_mult_sel >= 0 && mclk_mult_sel <= 3)
    clk_ctrl_val |= mclk_n3_mult[mclk_mult_sel].val << NAU8325_MCLK_SEL_SFT; // N3

  clk_ctrl_val |= 0x1000; //  DAC_CLK = MCLK/2 (required!)

  writeRegister(NAU8325_R03_CLK_CTRL, clk_ctrl_val);

  writeRegister(NAU8325_R03_CLK_CTRL, clk_ctrl_val); // Now written only once

  // Configure ANALOG_CONTROL_5 for 4x/8x clock enable
  switch (mclk_mult_sel)
  {
  case 2:
    writeRegisterBits(NAU8325_R65_ANALOG_CONTROL_5,
                      NAU8325_MCLK4XEN_EN,
                      NAU8325_MCLK4XEN_EN);
    break;
  case 3:
    writeRegisterBits(NAU8325_R65_ANALOG_CONTROL_5,
                      NAU8325_MCLK4XEN_EN | NAU8325_MCLK8XEN_EN,
                      NAU8325_MCLK4XEN_EN | NAU8325_MCLK8XEN_EN);
    break;
  default:
    writeRegisterBits(NAU8325_R65_ANALOG_CONTROL_5,
                      NAU8325_MCLK4XEN_EN | NAU8325_MCLK8XEN_EN,
                      0);
    break;
  }

  return true;
}

int PCBCUPID_NAU8325::getMclkRatioAndN2Index(const SRateAttr *srate, int mclk_hz, int &n2_sel_out)
{
  int ratio = NAU8325_MCLK_FS_RATIO_NUM; // Default: not found

  for (int i = 0; i < 5; i++)
  { // mclk_n2_div[] has 5 entries
    int div = mclk_n2_div[i].param;
    int mclk_src = mclk_hz >> div;

    if (srate->mclk_src[NAU8325_MCLK_FS_RATIO_256] == mclk_src)
    {
      ratio = NAU8325_MCLK_FS_RATIO_256;
      n2_sel_out = i;
      break;
    }
    if (srate->mclk_src[NAU8325_MCLK_FS_RATIO_400] == mclk_src)
    {
      ratio = NAU8325_MCLK_FS_RATIO_400;
      n2_sel_out = i;
      break;
    }
    if (srate->mclk_src[NAU8325_MCLK_FS_RATIO_500] == mclk_src)
    {
      ratio = NAU8325_MCLK_FS_RATIO_500;
      n2_sel_out = i;
      break;
    }
  }

  return ratio;
}

const SRateAttr *PCBCUPID_NAU8325::getSRateAttr(int fs)

{
  for (size_t i = 0; i < sizeof(target_srate_table) / sizeof(target_srate_table[0]); ++i)
  {
    if (target_srate_table[i].fs == fs)
    {
      return &target_srate_table[i];
    }
  }
  return nullptr;
}

bool PCBCUPID_NAU8325::chooseClockSource(int fs, int mclk,
                                         const SRateAttr *&srate,
                                         int &n1_sel, int &mult_sel, int &n2_sel)
{
  if (fs <= 0 || mclk <= 0)
  {
    log_i("[NAU8325] Invalid fs or mclk.");
    return false;
  }

  srate = getSRateAttr(fs);
  if (!srate)
  {
    log_i("[NAU8325] Unsupported fs: %d\n", fs);
    return false;
  }

  // First try direct N2 mapping
  int ratio = getMclkRatioAndN2Index(srate, mclk, n2_sel);
  if (ratio != NAU8325_MCLK_FS_RATIO_NUM)
  {
    n1_sel = 0;
    mult_sel = -1; // Bypass
    log_i("[NAU8325] Direct match: fs=%d, mclk=%d → N2=%d (Ratio=%d)\n",
          fs, mclk, n2_sel, ratio);
    return true;
  }

  // Try all N1 and N3 combinations
  int mclk_max = 0;
  int best_n1 = -1, best_mult = -1, best_n2 = -1;

  for (int i = 0; i < 3; ++i) // N1 options
  {
    for (int j = 0; j < 4; ++j) // N3 multiplier options
    {
      int m = (mclk << mclk_n3_mult[j].param) / mclk_n1_div[i].param;
      int r = getMclkRatioAndN2Index(srate, m, n2_sel);

      if (r != NAU8325_MCLK_FS_RATIO_NUM && (m > mclk_max || best_n1 < i))
      {
        mclk_max = m;
        best_n1 = i;
        best_mult = j;
        best_n2 = n2_sel;
      }
    }
  }

  if (mclk_max > 0)
  {
    n1_sel = best_n1;
    mult_sel = best_mult;
    n2_sel = best_n2;

    log_i("[NAU8325] Matched via N1/N3 combo: fs=%d, mclk=%d → N1=%d, N3(mult)=%d, N2=%d\n",
          fs, mclk, n1_sel, mult_sel, n2_sel);
    log_i("[NAU8325] chooseClockSource(): fs=%d, mclk=%d\n", fs, mclk);

    return true;
  }

  log_i("[NAU8325] Failed to match MCLK %d Hz with fs %d Hz\n", mclk, fs);
  return false;
}

bool PCBCUPID_NAU8325::configureClocks(int fs, int mclk)
{
  const SRateAttr *srate = nullptr;
  int n1_sel = 0, mult_sel = -1, n2_sel = 0;

  if (!chooseClockSource(fs, mclk, srate, n1_sel, mult_sel, n2_sel))
  {
    log_i("[NAU8325] Failed to choose clock source for fs=%d, mclk=%d\n", fs, mclk);
    return false;
  }

  if (!applySampleRateClocks(srate, n1_sel, mult_sel, n2_sel))
  {
    log_i("[NAU8325] Failed to apply sample rate clocks.");
    return false;
  }
  log_i("[NAU8325] configureClocks(): fs=%u, mclk=%u\n", fs, mclk);

  return true;
}

const OsrAttr *PCBCUPID_NAU8325::getCurrentOSR()
{
  uint16_t value;
  if (!readRegister(NAU8325_R29_DAC_CTRL1, value))
  {
    return nullptr;
  }

  uint8_t osr_index = value & NAU8325_DAC_OVERSAMPLE_MASK;

  if (osr_index >= sizeof(osr_dac_sel) / sizeof(osr_dac_sel[0]))
  {
    return nullptr;
  }

  return &osr_dac_sel[osr_index];
}

bool PCBCUPID_NAU8325::configureAudio(uint32_t fs, uint32_t mclk, uint8_t bits_per_sample)
{
  this->fs = fs;
  this->mclk = mclk;

  // Configure clock tree
  if (!configureClocks(fs, mclk))
  {
    return false;
  }

  //  Optional: set oversampling mode explicitly (OSR = 128)
  writeRegisterBits(NAU8325_R29_DAC_CTRL1, NAU8325_DAC_OVERSAMPLE_MASK, NAU8325_DAC_OVERSAMPLE_128);

  const OsrAttr *osr = getCurrentOSR();
  if (!osr || osr->osr == 0 || (fs * osr->osr > CLK_DA_AD_MAX))
  {
    log_i("[NAU8325] Invalid OSR or fs × OSR exceeds max.");
    return false;
  }

  // Set DAC clock source
  writeRegisterBits(NAU8325_R03_CLK_CTRL,
                    NAU8325_CLK_DAC_SRC_MASK,
                    osr->clk_src << NAU8325_CLK_DAC_SRC_SFT);

  // Configure I2S word length
  uint16_t val_len = 0;
  switch (bits_per_sample)
  {
  case 16:
    val_len = NAU8325_I2S_DL_16;
    break;
  case 20:
    val_len = NAU8325_I2S_DL_20;
    break;
  case 24:
    val_len = NAU8325_I2S_DL_24;
    break;
  case 32:
    val_len = NAU8325_I2S_DL_32;
    break;
  default:
    log_i("[NAU8325] Invalid bit width: %u\n", bits_per_sample);
    return false;
  }

  writeRegisterBits(NAU8325_R0D_I2S_PCM_CTRL1,
                    NAU8325_I2S_DL_MASK, val_len);

  log_i("[NAU8325] configureAudio(): fs=%u, mclk=%u, bits=%u\n", fs, mclk, bits_per_sample);

  return true;
}

bool PCBCUPID_NAU8325::setI2SFormat(AudioI2SFormat format, ClockPolarity polarity)
{
  uint16_t ctrl1_val = 0;

  // Set BCLK polarity
  if (polarity == INVERTED_BCLK)
  {
    ctrl1_val |= NAU8325_I2S_BP_INV;
  }

  // Set data format
  switch (format)
  {
  case I2S_STD:
    ctrl1_val |= NAU8325_I2S_DF_I2S;
    break;
  case LEFT_JUSTIFIED:
    ctrl1_val |= NAU8325_I2S_DF_LEFT;
    break;
  case RIGHT_JUSTIFIED:
    ctrl1_val |= NAU8325_I2S_DF_RIGTH;
    break;
  case DSP_A:
    ctrl1_val |= NAU8325_I2S_DF_PCM_AB;
    break;
  case DSP_B:
    ctrl1_val |= NAU8325_I2S_DF_PCM_AB | NAU8325_I2S_PCMB_EN;
    break;
  default:
    return false;
  }

  // Apply to I2S_PCM_CTRL1
  return writeRegisterBits(NAU8325_R0D_I2S_PCM_CTRL1,
                           NAU8325_I2S_DF_MASK | NAU8325_I2S_BP_MASK | NAU8325_I2S_PCMB_EN,
                           ctrl1_val);
}

bool PCBCUPID_NAU8325::setSysClock(uint32_t freq)
{
  if (freq < MASTER_CLK_MIN || freq > MASTER_CLK_MAX)
  {
    log_i("[NAU8325] Invalid MCLK: %u Hz (allowed: %u - %u)\n", freq, MASTER_CLK_MIN, MASTER_CLK_MAX);
    return false;
  }

  this->mclk = freq;
  log_i("[NAU8325] MCLK set to %u Hz\n", mclk);
  return true;
}

void PCBCUPID_NAU8325::setPowerUpDefault(bool enable)
{
  writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, NAU8325_PWRUP_DFT,
                    enable ? NAU8325_PWRUP_DFT : 0);
}

void PCBCUPID_NAU8325::setOversampling(OversamplingMode mode)
{
  writeRegisterBits(NAU8325_R29_DAC_CTRL1, NAU8325_DAC_OVERSAMPLE_MASK, mode);
}

void PCBCUPID_NAU8325::setVolume(uint8_t left, uint8_t right)
{
  uint16_t value = ((left & 0xFF) << 8) | (right & 0xFF);
  writeRegister(NAU8325_R13_DAC_VOLUME, value);
}

void PCBCUPID_NAU8325::softMute(bool enable)
{
  writeRegisterBits(NAU8325_R12_MUTE_CTRL, NAU8325_SOFT_MUTE,
                    enable ? NAU8325_SOFT_MUTE : 0);
  delay(30);
}

void PCBCUPID_NAU8325::initRegisters()
{
  // Set ALC parameters default timing and max gain
  writeRegisterBits(NAU8325_R2C_ALC_CTRL1, NAU8325_ALC_MAXGAIN_MASK, 0x7 << NAU8325_ALC_MAXGAIN_SFT);
  writeRegisterBits(NAU8325_R2D_ALC_CTRL2,
                    NAU8325_ALC_DCY_MASK | NAU8325_ALC_ATK_MASK | NAU8325_ALC_HLD_MASK,
                    (0x5 << NAU8325_ALC_DCY_SFT) | (0x3 << NAU8325_ALC_ATK_SFT) | (0x5 << NAU8325_ALC_HLD_SFT));

  /* Enable ALC to avoid signal distortion when battery low. */
  if (alc_enable)
  {
    writeRegisterBits(NAU8325_R2E_ALC_CTRL3, NAU8325_ALC_EN, NAU8325_ALC_EN);
  }

  // Clock detection bits
  uint16_t clk_det_mask = NAU8325_CLKPWRUP_DIS | NAU8325_PWRUP_DFT;

  if (clock_detection)
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, clk_det_mask, 0);
  else
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, clk_det_mask, NAU8325_CLKPWRUP_DIS);

  if (clock_det_data)
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, NAU8325_APWRUP_EN, NAU8325_APWRUP_EN);
  else
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, NAU8325_APWRUP_EN, 0);

  // DAC Vref configuration
  uint16_t vref_val = 0xFFFF;
  if (dac_vref_microvolt == 1800000)
    vref_val = 0 << NAU8325_DACVREFSEL_SFT;
  else if (dac_vref_microvolt == 2700000)
    vref_val = 1 << NAU8325_DACVREFSEL_SFT;
  else if (dac_vref_microvolt == 2880000)
    vref_val = 2 << NAU8325_DACVREFSEL_SFT;
  else if (dac_vref_microvolt == 3060000)
    vref_val = 3 << NAU8325_DACVREFSEL_SFT;

  if (vref_val != 0xFFFF)
  {
    writeRegisterBits(NAU8325_R73_RDAC, NAU8325_DACVREFSEL_MASK, vref_val);
  }
  else
  {
    log_i("[NAU8325] Invalid DAC Vref: %lu uV\n", dac_vref_microvolt);
  }

  /* DAC Reference Voltage Decoupling Capacitors. */
  writeRegisterBits(NAU8325_R63_ANALOG_CONTROL_3, NAU8325_CLASSD_COARSE_GAIN_MASK, 0x4);
  /* Auto-Att Min Gain 0dB, Class-D N Driver Slew Rate -25%. */
  writeRegisterBits(NAU8325_R64_ANALOG_CONTROL_4, NAU8325_CLASSD_SLEWN_MASK, 0x7);

  // VMID resistor selection
  uint16_t vref_imp_val = 0xFFFF;
  if (vref_impedance_ohms == 0)
    vref_imp_val = 0 << NAU8325_BIAS_VMID_SEL_SFT;
  else if (vref_impedance_ohms == 25000)
    vref_imp_val = 1 << NAU8325_BIAS_VMID_SEL_SFT;
  else if (vref_impedance_ohms == 125000)
    vref_imp_val = 2 << NAU8325_BIAS_VMID_SEL_SFT;
  else if (vref_impedance_ohms == 2500)
    vref_imp_val = 3 << NAU8325_BIAS_VMID_SEL_SFT;

  if (vref_imp_val != 0xFFFF)
  {
    writeRegisterBits(NAU8325_R60_BIAS_ADJ, NAU8325_BIAS_VMID_SEL_MASK, vref_imp_val);
  }
  else
  {
    log_i("[NAU8325] Invalid VMID impedance: %lu ohms\n", vref_impedance_ohms);
  }

  // Enable VMID, BIAS, DACs, etc., Voltage / current Amps
  writeRegisterBits(NAU8325_R61_ANALOG_CONTROL_1,
                    NAU8325_DACEN_MASK | NAU8325_DACCLKEN_MASK | NAU8325_DACEN_R_MASK | NAU8325_DACCLKEN_R_MASK | NAU8325_CLASSDEN_MASK | NAU8325_VMDFSTENB_MASK | NAU8325_BIASEN_MASK | NAU8325_VMIDEN_MASK,
                    (0x1 << NAU8325_DACEN_SFT) | (0x1 << NAU8325_DACCLKEN_SFT) | (0x1 << NAU8325_DACEN_R_SFT) | (0x1 << NAU8325_DACCLKEN_R_SFT) | (0x1 << NAU8325_CLASSDEN_SFT) | (0x1 << NAU8325_VMDFSTENB_SFT) | (0x1 << NAU8325_BIASEN_SFT) | (1 << NAU8325_VMIDEN_SFT));

  /* Enable ALC to avoid signal distortion when battery low. */
  if (alc_enable)
  {
    writeRegisterBits(NAU8325_R2E_ALC_CTRL3, NAU8325_ALC_EN, NAU8325_ALC_EN);
  }

  // Clock detection bits
  uint16_t clk_det_mask1 = NAU8325_CLKPWRUP_DIS | NAU8325_PWRUP_DFT;

  if (clock_detection)
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, clk_det_mask1, 0);
  else
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, clk_det_mask1, NAU8325_CLKPWRUP_DIS);

  if (clock_det_data)
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, NAU8325_APWRUP_EN, NAU8325_APWRUP_EN);
  else
    writeRegisterBits(NAU8325_R40_CLK_DET_CTRL, NAU8325_APWRUP_EN, 0);

  // Set default OSR = 128
  writeRegisterBits(NAU8325_R29_DAC_CTRL1,
                    NAU8325_DAC_OVERSAMPLE_MASK,
                    NAU8325_DAC_OVERSAMPLE_128);
}

void PCBCUPID_NAU8325::powerOn()
{
  // Clear soft mute to allow audio output
  softMute(false);

  // Enable DACs, clocks, bias, class-D
  writeRegisterBits(NAU8325_R61_ANALOG_CONTROL_1,
                    NAU8325_DACEN_MASK | NAU8325_DACCLKEN_MASK | NAU8325_DACEN_R_MASK | NAU8325_DACCLKEN_R_MASK | NAU8325_CLASSDEN_MASK | NAU8325_VMDFSTENB_MASK | NAU8325_BIASEN_MASK | NAU8325_VMIDEN_MASK,
                    (3 << NAU8325_DACEN_SFT) | (3 << NAU8325_DACCLKEN_SFT) | (3 << NAU8325_DACEN_R_SFT) | (3 << NAU8325_DACCLKEN_R_SFT) | (3 << NAU8325_CLASSDEN_SFT) | (3 << NAU8325_VMDFSTENB_SFT) | (3 << NAU8325_BIASEN_SFT) | 0x03); // VMID_EN

  // If clock detection is disabled, enable PWRUP_DFT
  setPowerUpDefault(true);

  delay(30); // Let analog circuitry stabilize
}

void PCBCUPID_NAU8325::powerOff()
{
  // Soft mute first to prevent pop
  softMute(true);

  // Disable analog and DAC power
  writeRegisterBits(NAU8325_R61_ANALOG_CONTROL_1,
                    NAU8325_DACEN_MASK | NAU8325_DACCLKEN_MASK | NAU8325_DACEN_R_MASK | NAU8325_DACCLKEN_R_MASK | NAU8325_CLASSDEN_MASK | NAU8325_VMDFSTENB_MASK | NAU8325_BIASEN_MASK | NAU8325_VMIDEN_MASK,
                    0x0000);

  // If clock detection is disabled, clear PWRUP_DFT
  setPowerUpDefault(false);

  delay(30); // Let output fade out
}

bool PCBCUPID_NAU8325::writeRegister(uint16_t reg, uint16_t val)
{
  log_i("[WRITE] reg=0x%04X val=0x%04X\n", reg, val);
  i2c.beginTransmission(i2c_addr);
  i2c.write((reg >> 8) & 0xFF);
  i2c.write(reg & 0xFF);
  i2c.write((val >> 8) & 0xFF);
  i2c.write(val & 0xFF);
  return i2c.endTransmission() == 0;
}

bool PCBCUPID_NAU8325::writeRegisterBits(uint16_t reg, uint16_t mask, uint16_t value) // mask ---> is to set the the particular bit renaining bit untouched.
{
  uint16_t current;
  if (!readRegister(reg, current))
    return false;

  uint16_t new_value = (current & ~mask) | (value & mask);
  return writeRegister(reg, new_value);
}

bool PCBCUPID_NAU8325::readRegister(uint16_t reg, uint16_t &value)
{
  i2c.beginTransmission(i2c_addr);
  i2c.write((reg >> 8) & 0xFF);
  i2c.write(reg & 0xFF);
  if (i2c.endTransmission(false) != 0)
  {
    return false;
  }

  if (i2c.requestFrom(i2c_addr, (uint8_t)2) != 2)
  {
    return false;
  }

  uint8_t high = i2c.read();
  uint8_t low = i2c.read();
  value = ((uint16_t)high << 8) | low;
  return true;
}