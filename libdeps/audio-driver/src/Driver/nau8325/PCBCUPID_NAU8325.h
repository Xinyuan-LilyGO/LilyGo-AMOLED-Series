#ifndef PCBCUPID_NAU8325_H
#define PCBCUPID_NAU8325_H

#include <Arduino.h>
#include <Wire.h>

#define NAU8325_I2C_ADDR 0x21

#define NAU8325_R00_HARDWARE_RST 0x00
#define NAU8325_R01_SOFTWARE_RST 0x01
#define NAU8325_R02_DEVICE_ID 0x02
#define NAU8325_R03_CLK_CTRL 0x03
#define NAU8325_R04_ENA_CTRL 0x04
#define NAU8325_R05_INTERRUPT_CTRL 0x05
#define NAU8325_R06_INT_CLR_STATUS 0x06
#define NAU8325_R09_IRQOUT 0x09
#define NAU8325_R0A_IO_CTRL 0x0a
#define NAU8325_R0B_PDM_CTRL 0x0b
#define NAU8325_R0C_TDM_CTRL 0x0c
#define NAU8325_R0D_I2S_PCM_CTRL1 0x0d
#define NAU8325_R0E_I2S_PCM_CTRL2 0x0e
#define NAU8325_R0F_L_TIME_SLOT 0x0f
#define NAU8325_R10_R_TIME_SLOT 0x10
#define NAU8325_R11_HPF_CTRL 0x11
#define NAU8325_R12_MUTE_CTRL 0x12
#define NAU8325_R13_DAC_VOLUME 0x13
#define NAU8325_R1D_DEBUG_READ1 0x1d
#define NAU8325_R1F_DEBUG_READ2 0x1f
#define NAU8325_R22_DEBUG_READ3 0x22
#define NAU8325_R29_DAC_CTRL1 0x29
#define NAU8325_R2A_DAC_CTRL2 0x2a
#define NAU8325_R2C_ALC_CTRL1 0x2c
#define NAU8325_R2D_ALC_CTRL2 0x2d
#define NAU8325_R2E_ALC_CTRL3 0x2e
#define NAU8325_R2F_ALC_CTRL4 0x2f
#define NAU8325_R40_CLK_DET_CTRL 0x40
#define NAU8325_R49_TEST_STATUS 0x49
#define NAU8325_R4A_ANALOG_READ 0x4a
#define NAU8325_R50_MIXER_CTRL 0x50
#define NAU8325_R55_MISC_CTRL 0x55
#define NAU8325_R60_BIAS_ADJ 0x60
#define NAU8325_R61_ANALOG_CONTROL_1 0x61
#define NAU8325_R62_ANALOG_CONTROL_2 0x62
#define NAU8325_R63_ANALOG_CONTROL_3 0x63
#define NAU8325_R64_ANALOG_CONTROL_4 0x64
#define NAU8325_R65_ANALOG_CONTROL_5 0x65
#define NAU8325_R66_ANALOG_CONTROL_6 0x66
#define NAU8325_R69_CLIP_CTRL 0x69
#define NAU8325_R73_RDAC 0x73
#define NAU8325_REG_MAX NAU8325_R73_RDAC

#define NAU8325_VMIDEN_SFT 0x03

/* 16-bit control register address, and 16-bits control register data */
#define NAU8325_REG_ADDR_LEN 16
#define NAU8325_REG_DATA_LEN 16

/*Below are bit masking of individual register*/

/* CLK_CTRL (0x03) */
#define NAU8325_CLK_DAC_SRC_SFT 12
#define NAU8325_CLK_DAC_SRC_MASK (0x3 << NAU8325_CLK_DAC_SRC_SFT)
#define NAU8325_CLK_MUL_SRC_SFT 6
#define NAU8325_CLK_MUL_SRC_MASK (0x3 << NAU8325_CLK_MUL_SRC_SFT)
#define NAU8325_MCLK_SEL_SFT 3
#define NAU8325_MCLK_SEL_MASK (0x7 << NAU8325_MCLK_SEL_SFT)
#define NAU8325_MCLK_SRC_MASK 0x7

/* ENA_CTRL (0x04) */
#define NAU8325_DAC_LEFT_CH_EN_SFT 3
#define NAU8325_DAC_LEFT_CH_EN (0x1 << NAU8325_DAC_LEFT_CH_EN_SFT)
#define NAU8325_DAC_RIGHT_CH_EN_SFT 2
#define NAU8325_DAC_RIGHT_CH_EN (0x1 << NAU8325_DAC_RIGHT_CH_EN_SFT)

// R04: ENA_CTRL
#define NAU8325_ENA_CTRL_DACR_EN 0x0008     // Bit 3
#define NAU8325_ENA_CTRL_DACL_EN 0x0004     // Bit 2
#define NAU8325_ENA_CTRL_DAC_EN_MASK 0x000C // Bits 3:2

/* INTERRUPT_CTRL (0x05) */
#define NAU8325_ARP_DWN_INT_SFT 12
#define NAU8325_ARP_DWN_INT_MASK (0x1 << NAU8325_ARP_DWN_INT_SFT)
#define NAU8325_CLIP_INT_SFT 11
#define NAU8325_CLIP_INT_MASK (0x1 << NAU8325_CLIP_INT_SFT)
#define NAU8325_LVD_INT_SFT 10
#define NAU8325_LVD_INT_MASK (0x1 << NAU8325_LVD_INT_SFT)
#define NAU8325_PWR_INT_DIS_SFT 8
#define NAU8325_PWR_INT_DIS (0x1 << NAU8325_PWR_INT_DIS_SFT)
#define NAU8325_OCP_OTP_SHTDWN_INT_SFT 4
#define NAU8325_OCP_OTP_SHTDWN_INT_MASK (0x1 << NAU8325_OCP_OTP_SHTDWN_INT_SFT)
#define NAU8325_CLIP_INT_DIS_SFT 3
#define NAU8325_CLIP_INT_DIS (0x1 << NAU8325_CLIP_INT_DIS_SFT)
#define NAU8325_LVD_INT_DIS_SFT 2
#define NAU8325_LVD_INT_DIS (0x1 << NAU8325_LVD_INT_DIS_SFT)
#define NAU8325_PWR_INT_MASK 0x1

/* INT_CLR_STATUS (0x06) */
#define NAU8325_INT_STATUS_MASK 0x7f

/* IRQOUT (0x9) */
#define NAU8325_IRQOUT_SEL_SEF 12
#define NAU8325_IRQOUT_SEL_MASK (0xf << NAU8325_IRQOUT_SEL_SEF)
#define NAU8325_DEM_DITH_SFT 7
#define NAU8325_DEM_DITH_EN (0x1 << NAU8325_DEM_DITH_SFT)
#define NAU8325_GAINZI3_SFT 5
#define NAU8325_GAINZI3_MASK (0x1 << NAU8325_GAINZI3_SFT)
#define NAU8325_GAINZI2_MASK 0x1f

/* IO_CTRL (0x0a) */
#define NAU8325_IRQ_PL_SFT 15
#define NAU8325_IRQ_PL_ACT_HIGH (0x1 << NAU8325_IRQ_PL_SFT)
#define NAU8325_IRQ_PS_SFT 14
#define NAU8325_IRQ_PS_UP (0x1 << NAU8325_IRQ_PS_SFT)
#define NAU8325_IRQ_PE_SFT 13
#define NAU8325_IRQ_PE_EN (0x1 << NAU8325_IRQ_PE_SFT)
#define NAU8325_IRQ_DS_SFT 12
#define NAU8325_IRQ_DS_HIGH (0x1 << NAU8325_IRQ_DS_SFT)
#define NAU8325_IRQ_OUTPUT_SFT 11
#define NAU8325_IRQ_OUTPUT_EN (0x1 << NAU8325_IRQ_OUTPUT_SFT)
#define NAU8325_IRQ_PIN_DEBUG_SFT 10
#define NAU8325_IRQ_PIN_DEBUG_EN (0x1 << NAU8325_IRQ_PIN_DEBUG_SFT)

/* PDM_CTRL (0x0b) */
#define NAU8325_PDM_LCH_EDGE_SFT 1
#define NAU8325_PDM_LCH_EDGE__MASK (0x1 << NAU8325_PDM_LCH_EDGE_SFT)
#define NAU8325_PDM_MODE_EN 0x1

/* TDM_CTRL (0x0c) */
#define NAU8325_TDM_SFT 15
#define NAU8325_TDM_EN (0x1 << NAU8325_TDM_SFT)
#define NAU8325_PCM_OFFSET_CTRL_SFT 14
#define NAU8325_PCM_OFFSET_CTRL_EN (0x1 << NAU8325_PCM_OFFSET_CTRL_SFT)
#define NAU8325_DAC_LEFT_SFT 6
#define NAU8325_NAU8325_DAC_LEFT_MASK (0x7 << NAU8325_DAC_LEFT_SFT)
#define NAU8325_DAC_RIGHT_SFT 3
#define NAU8325_DAC_RIGHT_MASK (0x7 << NAU8325_DAC_RIGHT_SFT)

/* I2S_PCM_CTRL1 (0x0d) */
#define NAU8325_DACCM_CTL_SFT 14
#define NAU8325_DACCM_CTL_MASK (0x3 << NAU8325_DACCM_CTL_SFT)
#define NAU8325_CMB8_0_SFT 10
#define NAU8325_CMB8_0_MASK (0x1 << NAU8325_CMB8_0_SFT)
#define NAU8325_UA_OFFSET_SFT 9
#define NAU8325_UA_OFFSET_MASK (0x1 << NAU8325_UA_OFFSET_SFT)
#define NAU8325_I2S_BP_SFT 7
#define NAU8325_I2S_BP_MASK (0x1 << NAU8325_I2S_BP_SFT)
#define NAU8325_I2S_BP_INV (0x1 << NAU8325_I2S_BP_SFT)
#define NAU8325_I2S_PCMB_SFT 6
#define NAU8325_I2S_PCMB_EN (0x1 << NAU8325_I2S_PCMB_SFT)
#define NAU8325_I2S_DACPSHS0_SFT 5
#define NAU8325_I2S_DACPSHS0_MASK (0x1 << NAU8325_I2S_DACPSHS0_SFT)
#define NAU8325_I2S_DL_SFT 2
#define NAU8325_I2S_DL_MASK (0x3 << NAU8325_I2S_DL_SFT)
#define NAU8325_I2S_DL_32 (0x3 << NAU8325_I2S_DL_SFT)
#define NAU8325_I2S_DL_24 (0x2 << NAU8325_I2S_DL_SFT)
#define NAU8325_I2S_DL_20 (0x1 << NAU8325_I2S_DL_SFT)
#define NAU8325_I2S_DL_16 (0x0 << NAU8325_I2S_DL_SFT)
#define NAU8325_I2S_DF_MASK 0x3
#define NAU8325_I2S_DF_RIGTH 0x0
#define NAU8325_I2S_DF_LEFT 0x1
#define NAU8325_I2S_DF_I2S 0x2
#define NAU8325_I2S_DF_PCM_AB 0x3

/* I2S_PCM_CTRL2 (0x0e) */
#define NAU8325_PCM_TS_SFT 10
#define NAU8325_PCM_TS_EN (0x1 << NAU8325_PCM_TS_SFT)
#define NAU8325_PCM8BIT0_SFT 8
#define NAU8325_PCM8BIT0_MASK (0x1 << NAU8325_PCM8BIT0_SFT)

/* L_TIME_SLOT (0x0f)*/
#define NAU8325_SHORT_FS_DET_SFT 13
#define NAU8325_SHORT_FS_DET_DIS (0x1 << NAU8325_SHORT_FS_DET_SFT)
#define NAU8325_TSLOT_L0_MASK 0x3ff

/* R_TIME_SLOT (0x10)*/
#define NAU8325_TSLOT_R0_MASK 0x3ff

/* HPF_CTRL (0x11)*/
#define NAU8325_DAC_HPF_SFT 15
#define NAU8325_DAC_HPF_EN (0x1 << NAU8325_DAC_HPF_SFT)
#define NAU8325_DAC_HPF_APP_SFT 14
#define NAU8325_DAC_HPF_APP_MASK (0x1 << NAU8325_DAC_HPF_APP_SFT)
#define NAU8325_DAC_HPF_FCUT_SFT 11
#define NAU8325_DAC_HPF_FCUT_MASK (0x7 << NAU8325_DAC_HPF_FCUT_SFT)

/* MUTE_CTRL (0x12)*/
#define NAU8325_SOFT_MUTE_SFT 15
#define NAU8325_SOFT_MUTE (0x1 << NAU8325_SOFT_MUTE_SFT)
#define NAU8325_DAC_ZC_SFT 8
#define NAU8325_DAC_ZC_EN (0x1 << NAU8325_DAC_ZC_SFT)
#define NAU8325_UNMUTE_CTL_SFT 6
#define NAU8325_UNMUTE_CTL_MASK (0x3 << NAU8325_UNMUTE_CTL_SFT)
#define NAU8325_ANA_MUTE_SFT 4
#define NAU8325_ANA_MUTE_MASK (0x3 << NAU8325_ANA_MUTE_SFT)
#define NAU8325_AUTO_MUTE_SFT 3
#define NAU8325_AUTO_MUTE_DIS (0x1 << NAU8325_AUTO_MUTE_SFT)

/* DAC_VOLUME (0x13) */
#define NAU8325_DAC_VOLUME_L_SFT 8
#define NAU8325_DAC_VOLUME_L_EN (0xff << NAU8325_DAC_VOLUME_L_SFT)
#define NAU8325_DAC_VOLUME_R_SFT 0
#define NAU8325_DAC_VOLUME_R_EN (0xff << NAU8325_DAC_VOLUME_R_SFT)
#define NAU8325_DAC_VOL_MAX 0xff

/* DEBUG_READ1 (0x1d)*/
#define NAU8325_OSR100_MASK (0x1 << 6)
#define NAU8325_MIPS500_MASK (0x1 << 5)
#define NAU8325_SHUTDWNDRVR_R_MASK (0x1 << 4)
#define NAU8325_SHUTDWNDRVR_L_MASK (0x1 << 3)
#define NAU8325_MUTEB_MASK (0x1 << 2)
#define NAU8325_PDOSCB_MASK (0x1 << 1)
#define NAU8325_POWERDOWN1B_D_MASK 0x1

/* DEBUG_READ2 (0x1f)*/
#define NAU8325_R_CHANNEL_Vol_SFT 8
#define NAU8325_R_CHANNEL_Vol_MASK (0xff << NAU8325_R_CHANNEL_Vol_SFT)
#define NAU8325_L_CHANNEL_Vol_MASK 0xff

/* DEBUG_READ3(0x22)*/
#define NAU8325_PGAL_GAIN_MASK (0x3f << 7)
#define NAU8325_CLIP_MASK (0x1 << 6)
#define NAU8325_SCAN_MODE_MASK (0x1 << 5)
#define NAU8325_SDB_MASK (0x1 << 4)
#define NAU8325_TALARM_MASK (0x1 << 3)
#define NAU8325_SHORTR_MASK (0x1 << 2)
#define NAU8325_SHORTL_MASK (0x1 << 1)
#define NAU8325_TMDET_MASK 0x1

/* DAC_CTRL1 (0x29) */
#define NAU8325_DAC_OVERSAMPLE_SFT 0
#define NAU8325_DAC_OVERSAMPLE_MASK 0x7
#define NAU8325_DAC_OVERSAMPLE_256 1
#define NAU8325_DAC_OVERSAMPLE_128 2
#define NAU8325_DAC_OVERSAMPLE_64 0
#define NAU8325_DAC_OVERSAMPLE_32 4

/* ALC_CTRL1 (0x2c) */
#define NAU8325_ALC_MAXGAIN_SFT 5
#define NAU8325_ALC_MAXGAIN_MAX 0x7
#define NAU8325_ALC_MAXGAIN_MASK (0x7 << NAU8325_ALC_MAXGAIN_SFT)
#define NAU8325_ALC_MINGAIN_MAX 4
#define NAU8325_ALC_MINGAIN_SFT 1
#define NAU8325_ALC_MINGAIN_MASK (0x7 << NAU8325_ALC_MINGAIN_SFT)

/* ALC_CTRL2 (0x2d) */
#define NAU8325_ALC_DCY_SFT 12
#define NAU8325_ALC_DCY_MAX 0xb
#define NAU8325_ALC_DCY_MASK (0xf << NAU8325_ALC_DCY_SFT)
#define NAU8325_ALC_ATK_SFT 8
#define NAU8325_ALC_ATK_MAX 0xb
#define NAU8325_ALC_ATK_MASK (0xf << NAU8325_ALC_ATK_SFT)
#define NAU8325_ALC_HLD_SFT 4
#define NAU8325_ALC_HLD_MAX 0xa
#define NAU8325_ALC_HLD_MASK (0xf << NAU8325_ALC_HLD_SFT)
#define NAU8325_ALC_LVL_SFT 0
#define NAU8325_ALC_LVL_MAX 0xf
#define NAU8325_ALC_LVL_MASK 0xf

/* ALC_CTRL3 (0x2e) */
#define NAU8325_ALC_EN_SFT 15
#define NAU8325_ALC_EN (0x1 << NAU8325_ALC_EN_SFT)

/* TEMP_COMP_CTRL (0x30) */
#define NAU8325_TEMP_COMP_ACT2_MASK 0xff

/* LPF_CTRL (0x33) */
#define NAU8325_LPF_IN1_EN_SFT 15
#define NAU8325_LPF_IN1_EN (0x1 << NAU8325_LPF_IN1_EN_SFT)
#define NAU8325_LPF_IN1_TC_SFT 11
#define NAU8325_LPF_IN1_TC_MASK (0xf << NAU8325_LPF_IN1_TC_SFT)
#define NAU8325_LPF_IN2_EN_SFT 10
#define NAU8325_LPF_IN2_EN (0x1 << NAU8325_LPF_IN2_EN_SFT)
#define NAU8325_LPF_IN2_TC_SFT 6
#define NAU8325_LPF_IN2_TC_MASK (0xf << NAU8325_LPF_IN2_TC_SFT)

/* CLK_DET_CTRL (0x40) */
#define NAU8325_APWRUP_SFT 15
#define NAU8325_APWRUP_EN (0x1 << NAU8325_APWRUP_SFT)
#define NAU8325_CLKPWRUP_SFT 14
#define NAU8325_CLKPWRUP_DIS (0x1 << NAU8325_CLKPWRUP_SFT)
#define NAU8325_PWRUP_DFT_SFT 13
#define NAU8325_PWRUP_DFT (0x1 << NAU8325_PWRUP_DFT_SFT)
#define NAU8325_REG_SRATE_SFT 10
#define NAU8325_REG_SRATE_MASK (0x7 << NAU8325_REG_SRATE_SFT)
#define NAU8325_REG_ALT_SRATE_SFT 9
#define NAU8325_REG_ALT_SRATE_EN (0x1 << NAU8325_REG_ALT_SRATE_SFT)
#define NAU8325_REG_DIV_MAX 0x1

/* BIAS_ADJ (0x60) */
#define NAU8325_BIAS_VMID_SEL_SFT 4
#define NAU8325_BIAS_VMID_SEL_MASK (0x3 << NAU8325_BIAS_VMID_SEL_SFT)

/* ANALOG_CONTROL_1 (0x61) */
#define NAU8325_VMDFSTENB_SFT 14
#define NAU8325_VMDFSTENB_MASK (0x3 << NAU8325_VMDFSTENB_SFT)
#define NAU8325_CLASSDEN_SFT 12
#define NAU8325_CLASSDEN_MASK (0x3 << NAU8325_CLASSDEN_SFT)
#define NAU8325_DACCLKEN_R_SFT 10
#define NAU8325_DACCLKEN_R_MASK (0x3 << NAU8325_DACCLKEN_R_SFT)
#define NAU8325_DACEN_R_SFT 8
#define NAU8325_DACEN_R_MASK (0x3 << NAU8325_DACEN_R_SFT)
#define NAU8325_DACCLKEN_SFT 6
#define NAU8325_DACCLKEN_MASK (0x3 << NAU8325_DACCLKEN_SFT)
#define NAU8325_DACEN_SFT 4
#define NAU8325_DACEN_MASK (0x3 << NAU8325_DACEN_SFT)
#define NAU8325_BIASEN_SFT 2
#define NAU8325_BIASEN_MASK (0x3 << NAU8325_BIASEN_SFT)
#define NAU8325_VMIDEN_MASK 0x3

// R61: ANALOG_CONTROL_1
#define NAU8325_ANALOG_CTRL1_VMID_EN 0x0001   // Bit 0
#define NAU8325_ANALOG_CTRL1_CLASSD_EN 0x0002 // Bit 1
#define NAU8325_ANALOG_CTRL1_EN_MASK 0x0003   // Bits 1:0

/* ANALOG_CONTROL_2 (0x62) */
#define NAU8325_PWMMOD_SFT 14
#define NAU8325_PWMMOD_MASK (0x1 << NAU8325_PWMMOD_SFT)
#define NAU8325_DACTEST_SFT 6
#define NAU8325_DACTEST_MASK (0x3 << NAU8325_DACTEST_SFT)
#define NAU8325_DACREFCAP_SFT 4
#define NAU8325_DACREFCAP_MASK (0x3 << NAU8325_DACREFCAP_SFT)

/* ANALOG_CONTROL_3 (0x63) */
#define NAU8325_POWER_DOWN_L_SFT 12
#define NAU8325_POWER_DOWN_L_MASK (0x3 << NAU8325_POWER_DOWN_L_SFT)
#define NAU8325_POWER_DOWN_R_SFT 11
#define NAU8325_POWER_DOWN_R_MASK (0x3 << NAU8325_DACREFCAP_SFT)
#define NAU8325_CLASSD_FINE_SFT 5
#define NAU8325_CLASSD_FINE_MASK (0x3 << NAU8325_CLASSD_FINE_SFT)
#define NAU8325_CLASSD_COARSE_GAIN_MASK 0xf

/* ANALOG_CONTROL_4 (0x64) */
#define NAU8325_CLASSD_OCPN_SFT 12
#define NAU8325_CLASSD_OCPN_MASK (0xf << NAU8325_CLASSD_OCPN_SFT)
#define NAU8325_CLASSD_OCPP_SFT 8
#define NAU8325_CLASSD_OCPP_MASK (0xf << NAU8325_CLASSD_OCPP_SFT)
#define NAU8325_CLASSD_SLEWN_MASK 0xff

/* ANALOG_CONTROL_5 (0x65) */
#define NAU8325_MCLK_RANGE_SFT 2
#define NAU8325_MCLK_RANGE_EN (0x1 << NAU8325_MCLK_RANGE_SFT)
#define NAU8325_MCLK8XEN_SFT 1
#define NAU8325_MCLK8XEN_EN (0x1 << NAU8325_MCLK8XEN_SFT)
#define NAU8325_MCLK4XEN_EN 0x1

/* ANALOG_CONTROL_6 (0x66) */
#define NAU8325_VBATLOW_SFT 4
#define NAU8325_VBATLOW_MASK (0x1 << NAU8325_VBATLOW_SFT)
#define NAU8325_VDDSPK_LIM_SFT 3
#define NAU8325_VDDSPK_LIM_EN (0x1 << NAU8325_VDDSPK_LIM_SFT)
#define NAU8325_VDDSPK_LIM_MASK 0x7

// R66: ANALOG_CONTROL_6
#define NAU8325_ANALOG_CTRL6_SPKL_EN 0x0020 // Bit 5
#define NAU8325_ANALOG_CTRL6_SPKR_EN 0x0010 // Bit 4
#define NAU8325_ANALOG_CTRL6_EN_MASK 0x0030 // Bits 5:4

/* CLIP_CTRL (0x69)*/
#define NAU8325_ANTI_CLIP_SFT 4
#define NAU8325_ANTI_CLIP_EN (0x1 << NAU8325_ANTI_CLIP_SFT)

/* RDAC (0x73) */
#define NAU8325_CLK_DAC_DELAY_SFT 4
#define NAU8325_CLK_DAC_DELAY_EN (0x7 << NAU8325_CLK_DAC_DELAY_SFT)
#define NAU8325_DACVREFSEL_SFT 2
#define NAU8325_DACVREFSEL_MASK (0x3 << NAU8325_DACVREFSEL_SFT)

#define CLK_DA_AD_MAX 6144000UL // or another max as per datasheet

struct SrcAttr
{
  int param;
  uint8_t val;
};

struct OsrAttr
{
  int osr;
  uint8_t clk_src;
};

struct SRateAttr
{
  int fs;
  uint8_t range;
  bool max;
  uint32_t mclk_src[3];
};

struct RegDefault
{
  uint16_t reg;
  uint16_t val;
};

enum AudioI2SFormat
{
  I2S_STD,
  LEFT_JUSTIFIED,
  RIGHT_JUSTIFIED,
  DSP_A,
  DSP_B
};
enum ClockPolarity
{
  NORMAL_BCLK,
  INVERTED_BCLK
};
enum OversamplingMode
{
  OSR_64 = 2,
  OSR_128 = 1,
  OSR_256 = 0,
  OSR_32 = 4
};

enum ClockRatio
{
  NAU8325_MCLK_FS_RATIO_256 = 0,
  NAU8325_MCLK_FS_RATIO_400 = 1,
  NAU8325_MCLK_FS_RATIO_500 = 2,
  NAU8325_MCLK_FS_RATIO_NUM = 3
};

class PCBCUPID_NAU8325
{
public:
  PCBCUPID_NAU8325(TwoWire &wire);

  static const RegDefault reg_defaults[];

  // Static constant lookup tables
  static const SrcAttr mclk_n1_div[];
  static const SrcAttr mclk_n2_div[];
  static const SrcAttr mclk_n3_mult[];
  static const OsrAttr osr_dac_sel[];
  static const SRateAttr target_srate_table[];

  /*
    @brief
    fs- frame rate in Hz (e.g., 8000, 16000, 44100, 48000)
    bits_per_sample- bits per sample (e.g., 16, 24)
    ratio- MCLK to FS ratio (e.g., 256, 384, 512, 640)
    @return
    true if initialization is successful, false otherwise
    @note
    The begin() function initializes the NAU8325 codec with the specified
    frame rate, bits per sample, and MCLK to FS ratio. It configures the codec
    for audio playback and sets up the necessary registers. The function returns
    true if the initialization is successful, or false if there is an error.
    The default values for bits_per_sample and ratio are 16 and 256, respectively.
    If only fs is provided, it uses the default values for bits_per_sample and ratio.
    If no parameters are provided, it uses default values for fs, bits_per_sample,
    and ratio.
    Example:
    PCBCUPID_NAU8325 codec;
    codec.begin(44100, 16, 256); // Initialize with 44.1kHz, 16 bits, and MCLK/FS ratio of 256
    codec.begin(16000, 24); // Initialize with 16kHz, 24 bits, and default MCLK/FS ratio of 256
    codec.begin(); // Initialize with default values (e.g., 8000Hz, 16 bits, 256 ratio)
    @note
    The begin() function must be called before using the codec for audio playback.
    It sets up the codec's internal registers and prepares it for audio processing.
    If the initialization fails, the codec will not function correctly, and audio playback
    may not work as expected.
  */

  bool begin(uint32_t fs, uint8_t bits_per_sample, uint16_t ratio);
  bool begin(uint32_t fs, uint8_t bits_per_sample);
  bool begin();

  uint16_t readDeviceId();
  void resetChip();

  void softMute(bool enable);
  void powerOn();
  void powerOff();

  void setPowerUpDefault(bool enable);
  void setOversampling(OversamplingMode mode);
  void setVolume(uint8_t left, uint8_t right); // public

private:
  TwoWire &i2c;
  uint8_t i2c_addr = NAU8325_I2C_ADDR;
  uint32_t fs;
  uint32_t mclk;

  bool clock_detection;

  void initRegisters(); // Initialize codec

  bool setSysClock(uint32_t freq); // setup MCLK for I2S
  bool configureAudio(uint32_t fs, uint32_t mclk, uint8_t bits_per_sample);
  bool chooseClockSource(int fs, int mclk, const SRateAttr *&srate, int &n1_sel, int &mult_sel, int &n2_sel);
  bool configureClocks(int fs, int mclk);
  bool applySampleRateClocks(const SRateAttr *srate, int n1_sel, int mclk_mult_sel, int n2_sel);
  int getMclkRatioAndN2Index(const SRateAttr *srate, int mclk_hz, int &n2_sel_out);
  const SRateAttr *getSRateAttr(int fs);
  const OsrAttr *getCurrentOSR();
  bool setI2SFormat(AudioI2SFormat format, ClockPolarity polarity);

  void enableDAPMBlocks(); // analog path power enable

  // I2C register controls
  bool readRegister(uint16_t reg, uint16_t &value);
  bool writeRegister(uint16_t reg, uint16_t value);
  bool writeRegisterBits(uint16_t reg, uint16_t mask, uint16_t value);
};

#endif // PCBCUPID_NAU8325_H
