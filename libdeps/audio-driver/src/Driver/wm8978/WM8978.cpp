#include "WM8978.h"

// WM8978 register value buffer zone (total 58 registers 0 to 57), occupies 116
// bytes of memory Because the IIC WM8978 operation does not support read
// operations, so save all the register values in the local Write WM8978
// register, synchronized to the local register values, register read, register
// directly back locally stored value. Note: WM8978 register value is 9, so use
// uint16_t storage.

static uint16_t REGVAL_TBL[58] = {
    0X0000, 0X0000, 0X0000, 0X0000, 0X0050, 0X0000, 0X0140, 0X0000, 0X0000,
    0X0000, 0X0000, 0X00FF, 0X00FF, 0X0000, 0X0100, 0X00FF, 0X00FF, 0X0000,
    0X012C, 0X002C, 0X002C, 0X002C, 0X002C, 0X0000, 0X0032, 0X0000, 0X0000,
    0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0038, 0X000B, 0X0032, 0X0000,
    0X0008, 0X000C, 0X0093, 0X00E9, 0X0000, 0X0000, 0X0000, 0X0000, 0X0003,
    0X0010, 0X0010, 0X0100, 0X0100, 0X0002, 0X0001, 0X0001, 0X0039, 0X0039,
    0X0039, 0X0039, 0X0001, 0X0001};

// WM8978 write register
// reg: register address
// val: the value to be written to the register
// Return value: 0, success;
// Other, error code
uint8_t WM8978::Write_Reg(uint8_t reg, uint16_t val) {
  // char buf[2];
  // buf[0] = (reg << 1) | ((val >> 8) & 0X01);
  // buf[1] = val & 0XFF;
  //  p_wire->beginTransmission(
  //      WM8978_ADDR);  // Send data to the slave with device number 4
  //  p_wire->write((const uint8_t*)buf, 2);
  //  p_wire->endTransmission();  // Stop sending
  uint8_t byte_val = val;
  i2c_bus_write_bytes(p_wire, address, &reg, 1, &byte_val, 1);
  REGVAL_TBL[reg] = val;  // Save register value to local
  return 0;
}

// WM8978 init
// Return value: 0, initialization is normal
// Other, error code
uint8_t WM8978::Init(void) {
  uint8_t res;
  res = Write_Reg(0, 0);  // Soft reset WM8978
  if (res) return 1;      // Failed to send command, WM8978 exception
  // The following are common settings
  Write_Reg(1, 0X1B);   // R1, MICEN is set to 1 (MIC enabled), BIASEN is set to
                        // 1 (simulator works), VMIDSEL[1:0] is set to: 11 (5K)
  Write_Reg(2, 0X1B0);  // R2, ROUT1, LOUT1 output enable (headphones can work),
                        // BOOSTENR, BOOSTENL enable
  Write_Reg(3, 0X6C);   // R3, LOUT2, ROUT2 output enable (speaker operation),
                        // RMIX, LMIX enable
  Write_Reg(6, 0);      // R6, MCLK is provided externally
  Write_Reg(43, 1 << 4);  // R43, INVROUT2 reverse, drives the speaker
  Write_Reg(
      47,
      1 << 8);  // R47 setting, PGABOOSTL, left channel MIC gets 20 times gain
  Write_Reg(
      48,
      1 << 8);  // R48 setting, PGABOOSTR, right channel MIC gets 20 times gain
  Write_Reg(49, 1 << 1);           // R49, TSDEN, turn on overheating protection
  Write_Reg(10, 1 << 3);           // R10, SOFTMUTE off, 128x sampling, best SNR
  Write_Reg(14, 1 << 3 | 1 << 8);  // R14,ADC 128x sampling rate and enable high
                                   // pass filter (3.7Hz cut-off)
  return 0;
}

// WM8978 read register
// Reads the value  of the local register buffer zone
// reg: Register Address
// Return Value: Register value
uint16_t WM8978::Read_Reg(uint8_t reg) { return REGVAL_TBL[reg]; }
// WM8978 DAC/ADC configuration
// adcen:adc enable(1)/disable(0)
// dacen: dac enable(1)/disable(0)
void WM8978::cfgADDA(uint8_t dacen, uint8_t adcen) {
  uint16_t regval;
  regval = WM8978::Read_Reg(3);  // Read R3
  if (dacen)
    regval |= 3 << 0;  // Set the lowest 2 bits of R3 to 1, turn on DACR&DACL
  else
    regval &= ~(
        3 << 0);  // Clear the lowest 2 bits of R3 to 0 and turn off DACR&DACL.
  Write_Reg(3, regval);          // Set R3
  regval = WM8978::Read_Reg(2);  // Read R2
  if (adcen)
    regval |= 3 << 0;  // Set the lowest 2 bits of R2 to 1, turn on ADCR&ADCL
  else
    regval &=
        ~(3 << 0);  // Clear the lowest 2 bits of R2 and turn off ADCR&ADCL.
  Write_Reg(2, regval);  // Set R2
}
// WM8978 input channel configuration
// micen:MIC on (1)/off (0)
// lineinen:Line In on (1)/off (0)
// auxen:aux on (1)/off (0)
void WM8978::cfgInput(uint8_t micen, uint8_t lineinen, uint8_t auxen) {
  uint16_t regval;
  regval = WM8978::Read_Reg(2);  // Read R2
  if (micen)
    regval |= 3 << 2;  // Turn on INPPGAENR, INPPGAENL (MIC's PGA amplification)
  else
    regval &= ~(3 << 2);  // Close INPPGAENR,INPPGAENL.
  Write_Reg(2, regval);   // Set R2

  regval = WM8978::Read_Reg(44);  // Read R44
  if (micen)
    regval |= 3 << 4 |
              3 << 0;  // Enable LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA.
  else
    regval &= ~(3 << 4 |
                3 << 0);  // Close LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA.
  Write_Reg(44, regval);  // Set R44

  if (lineinen)
    WM8978::setLINEINgain(5);  // LINE IN 0dB gain
  else
    WM8978::setLINEINgain(0);  // Close LINE IN
  if (auxen)
    WM8978::setAUXgain(7);  // AUX 6dB gain
  else
    WM8978::setAUXgain(0);  // Close AUX input
}
// WM8978 output configuration
// dacen: DAC output (playback) on (1)/off (0)
// bpsen: Bypass output (recording, including MIC, LINE IN, AUX, etc.) on
// (1)/off (0)
void WM8978::cfgOutput(uint8_t dacen, uint8_t bpsen) {
  uint16_t regval = 0;
  if (dacen) regval |= 1 << 0;  // DAC output enable
  if (bpsen) {
    regval |= 1 << 1;  // BYPASS enabled
    regval |= 5 << 2;  // 0dB gain
  }
  Write_Reg(50, regval);  // R50 setting
  Write_Reg(51, regval);  // R51 setting
}
// WM8978 MIC gain setting (excluding BOOST's 20dB, MIC-->gain of the ADC input
// part) gain:0~63, corresponding to -12dB~35.25dB, 0.75dB/Step
void WM8978::setMICgain(uint8_t gain) {
  gain &= 0X3F;
  Write_Reg(45, gain);           // R45, left channel PGA setting
  Write_Reg(46, gain | 1 << 8);  // R46, right channel PGA setting
}
// WM8978 L2/R2 (that is, Line In) gain setting (L2/R2-->gain of the ADC input
// part) gain:0~7, 0 means channel is disabled, 1~7, corresponds to -12dB~6dB,
// 3dB/Step
void WM8978::setLINEINgain(uint8_t gain) {
  uint16_t regval;
  gain &= 0X07;
  regval = WM8978::Read_Reg(47);      // Read R47
  regval &= ~(7 << 4);                // Clear the original settings
  Write_Reg(47, regval | gain << 4);  // Set R47
  regval = WM8978::Read_Reg(48);      // Read R48
  regval &= ~(7 << 4);                // Clear the original settings
  Write_Reg(48, regval | gain << 4);  // Set R48
}
// WM8978 AUXR, AUXL (PWM audio part) gain setting (AUXR/L-->gain of ADC input
// part) gain:0~7, 0 means channel is disabled, 1~7, corresponds to -12dB~6dB,
// 3dB/Step
void WM8978::setAUXgain(uint8_t gain) {
  uint16_t regval;
  gain &= 0X07;
  regval = WM8978::Read_Reg(47);      // Read R47
  regval &= ~(7 << 0);                // Clear the original settings
  Write_Reg(47, regval | gain << 0);  // Set R47
  regval = WM8978::Read_Reg(48);      // Read R48
  regval &= ~(7 << 0);                // Clear the original settings
  Write_Reg(48, regval | gain << 0);  // Set R48
}

// Set I2S working mode
// fmt:0,LSB(right-aligned);1,MSB(left-aligned);2,Philips standard,
// I2S;3,PCM/DSP; len: 0, 16 digits; 1, 20 digits; 2, 24 digits; 3, 32 digits;
void WM8978::cfgI2S(uint8_t fmt, uint8_t len) {
  fmt &= 0X03;
  len &= 0X03;                            // Limited range
  Write_Reg(4, (fmt << 3) | (len << 5));  // R4, WM8978 working mode setting
}

// Set the left and right channel volume of the headset
// voll: left channel volume (0~63)
// volr: right channel volume (0~63)
void WM8978::setHPvol(uint8_t voll, uint8_t volr) {
  voll &= 0X3F;
  volr &= 0X3F;                    // Limited range
  if (voll == 0) voll |= 1 << 6;   // When the volume is 0, mute directly
  if (volr == 0) volr |= 1 << 6;   // When the volume is 0, mute directly
  Write_Reg(52, voll);             // R52, headphone left channel volume setting
  Write_Reg(53, volr | (1 << 8));  // R53, headphone right channel volume
                                   // setting, updated synchronously (HPVU=1)
}
// Set speaker volume
// voll: left channel volume (0~63)
void WM8978::setSPKvol(uint8_t volx) {
  volx &= 0X3F;                    // Limited range
  if (volx == 0) volx |= 1 << 6;   // When the volume is 0, mute directly
  Write_Reg(54, volx);             // R54, speaker left channel volume setting
  Write_Reg(55, volx | (1 << 8));  // R55, speaker right channel volume setting,
                                   // updated synchronously (SPKVU=1)
}
// Set 3D surround sound
// depth:0~15 (3D intensity, 0 is the weakest, 15 is the strongest)
void WM8978::set3D(uint8_t depth) {
  depth &= 0XF;          // Limited range
  Write_Reg(41, depth);  // R41, 3D surround setting
}

// Set EQ/3D action direction
// dir:0, works in ADC
//  1, works in DAC (default)
void WM8978::set3Ddir(uint8_t dir) {
  uint16_t regval;
  regval = WM8978::Read_Reg(0X12);
  if (dir)
    regval |= 1 << 8;
  else
    regval &= ~(1 << 8);
  Write_Reg(18, regval);  // R18, the 9th bit of EQ1 controls the EQ/3D
                          // direction
}

// Set EQ1
// cfreq: cutoff frequency, 0~3, corresponding to: 80/105/135/175Hz
// gain: Gain, 0~24, corresponding to -12~+12dB
void WM8978::setEQ1(uint8_t cfreq, uint8_t gain) {
  uint16_t regval;
  cfreq &= 0X3;  // Limited range
  if (gain > 24) gain = 24;
  gain = 24 - gain;
  regval = WM8978::Read_Reg(18);
  regval &= 0X100;
  regval |= cfreq << 5;   // Set cutoff frequency
  regval |= gain;         // Set gain
  Write_Reg(18, regval);  // R18, EQ1 setting
}
// Set EQ2
// cfreq: center frequency, 0~3, corresponding to: 230/300/385/500Hz
// gain: Gain, 0~24, corresponding to -12~+12dB
void WM8978::setEQ2(uint8_t cfreq, uint8_t gain) {
  uint16_t regval = 0;
  cfreq &= 0X3;  // Limited range
  if (gain > 24) gain = 24;
  gain = 24 - gain;
  regval |= cfreq << 5;   // Set cutoff frequency
  regval |= gain;         // Set gain
  Write_Reg(19, regval);  // R19, EQ2 setting
}
// Set EQ3
// cfreq: center frequency, 0~3, corresponding to: 650/850/1100/1400Hz
// gain: Gain, 0~24, corresponding to -12~+12dB
void WM8978::setEQ3(uint8_t cfreq, uint8_t gain) {
  uint16_t regval = 0;
  cfreq &= 0X3;  // Limited range
  if (gain > 24) gain = 24;
  gain = 24 - gain;
  regval |= cfreq << 5;   // Set cutoff frequency
  regval |= gain;         // Set gain
  Write_Reg(20, regval);  // R20, EQ3 setting
}
// Set EQ4
// cfreq: center frequency, 0~3, corresponding to: 1800/2400/3200/4100Hz
// gain: Gain, 0~24, corresponding to -12~+12dB
void WM8978::setEQ4(uint8_t cfreq, uint8_t gain) {
  uint16_t regval = 0;
  cfreq &= 0X3;  // Limited range
  if (gain > 24) gain = 24;
  gain = 24 - gain;
  regval |= cfreq << 5;   // Set cutoff frequency
  regval |= gain;         // Set gain
  Write_Reg(21, regval);  // R21, EQ4 settings
}
// Set EQ5
// cfreq: center frequency, 0~3, corresponding to: 5300/6900/9000/11700Hz
// gain: Gain, 0~24, corresponding to -12~+12dB
void WM8978::setEQ5(uint8_t cfreq, uint8_t gain) {
  uint16_t regval = 0;
  cfreq &= 0X3;  // Limited range
  if (gain > 24) gain = 24;
  gain = 24 - gain;
  regval |= cfreq << 5;   // Set cutoff frequency
  regval |= gain;         // Set gain
  Write_Reg(22, regval);  // R22, EQ5 setting
}

void WM8978::setALC(uint8_t enable, uint8_t maxgain, uint8_t mingain) {
  uint16_t regval;

  if (maxgain > 7) maxgain = 7;
  if (mingain > 7) mingain = 7;

  regval = WM8978::Read_Reg(32);
  if (enable) regval |= (3 << 7);
  regval |= (maxgain << 3) | (mingain << 0);
  Write_Reg(32, regval);
}

void WM8978::setNoise(uint8_t enable, uint8_t gain) {
  uint16_t regval;

  if (gain > 7) gain = 7;

  regval = WM8978::Read_Reg(35);
  regval = (enable << 3);
  regval |= gain;         // Set gain
  Write_Reg(35, regval);  // R18, EQ1 setting
}

void WM8978::setHPF(uint8_t enable) {
  uint16_t regval;

  regval = WM8978::Read_Reg(14);
  regval &= ~(1 << 8);
  regval |= (enable << 8);
  Write_Reg(14, regval);  // R14,high pass filter
}

bool WM8978::begin() {
  // p_wire->beginTransmission(WM8978_ADDR);
  // const uint8_t error = p_wire->endTransmission();
  // if (error) {
  //   AD_LOGE("No WM8978 dac @ i2c address: 0x%X", WM8978_ADDR);
  //   return false;
  // }
  const int err = Init();
  if (err) {
    AD_LOGE("WM8978 init err: 0x%X", err);
    return false;
  }
  cfgI2S(2, 0);       // Philips 16bit
  cfgADDA(1, 1);      // Enable ADC DAC
  cfgInput(0, 0, 0);  // mic, linein, aux - Note: M5Stack node has only internal
                      // microphones connected
  setMICgain(0);
  setAUXgain(0);
  setLINEINgain(0);
  setSPKvol(0);    // 0-63
  setHPvol(0, 0);  // 0-63
  set3Ddir(0);
  setEQ1(0, 24);
  setEQ2(0, 24);
  setEQ3(0, 24);
  setEQ4(0, 24);
  setEQ5(0, 24);
  cfgOutput(1, 0);  // Output enabled, bypass disabled
  return true;
}

bool WM8978::begin(i2c_bus_handle_t p_wire, int address) {
  this->address = address;
  this->p_wire = p_wire;
  return begin();
}
