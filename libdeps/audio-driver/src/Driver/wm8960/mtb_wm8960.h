/***********************************************************************************************//**
 * \file mtb_wm8960.h
 *
 * Description: This file contains the function prototypes and constants used
 * in mtb_wm8960.c. This driver is intended for the WM8960 audio codec.
 *
 ***************************************************************************************************
 * \copyright
 * Copyright 2018-2021 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **************************************************************************************************/

#pragma once

/**
 * @defgroup wm8960 WM8960 Audio Codec
 * @brief Transmit Audio
 */

/**
 * Basic set of APIs for interacting with the WM8960 audio codec display.
 * This provides basic initialization and access to to the audio codec.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "DriverCommon.h"

#ifdef ARDUINO
#  include <Arduino.h>
#  include <Wire.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

#define WM8960_I2C_ADDRESS (0x1A)


/**
 * \{
 * @name WM8960_REG_LEFT_IN_VOL (0x00) and WM8960_REG_RIGHT_IN_VOL (0x01)
 * Bit definitions for the WM8960_REG_LEFT_IN_VOL
 * and WM8960_REG_RIGHT_IN_VOL register
 */
#define WM8960_LEFT_RIGHT_IN_VOL_IPVU           0x100u /**< Input PGA Volume Update */
#define WM8960_LEFT_RIGHT_IN_VOL_INMUTE         0x080u /**< Input PGA Analogue Mute */
#define WM8960_LEFT_RIGHT_IN_VOL_IZC_ZC         0x040u /**< Input PGA Zero Cross Detector */
#define WM8960_LEFT_RIGHT_IN_VOL_INVOL_0dB      0x017u /**< Input PGA Volume 0dB */
#define WM8960_LEFT_RIGHT_IN_VOL_INVOL_30dB     0x03Fu /**< Input PGA Volume 30dB */
/** \} WM8960_REG_LEFT_IN_VOL and WM8960_REG_RIGHT_IN_VOL */

/**
 * \{
 * @name WM8960_REG_LOUT1_VOL (0x02) and WM8960_REG_ROUT1_VOL (0x03)
 * Bit definitions for the WM8960_REG_LOUT1_VOL
 * and WM8960_REG_ROUT1_VOL register.
 */
#define WM8960_LOUT1_ROUT1_VOL_OUT1VU           0x100u /**< Headphone Output Volume Update */
#define WM8960_LOUT1_ROUT1_VOL_O1ZC_ZC          0x080u /**< Headphone Output Zero Cross Enable */
#define WM8960_LOUT1_ROUT1_VOL_OUT1VOL_0dB      0x079u /**< LOUT1 or ROUT1 Volume 0dB */
#define WM8960_LOUT1_ROUT1_VOL_OUT1VOL_6dB      0x07Fu /**< LOUT1 or ROUT1 Volume 6dB */
/** \} WM8960_REG_LOUT1_VOL and WM8960_REG_ROUT1_VOL */

/**
 * \{
 * @name WM8960_REG_CLK1 (0x04)
 * Bit definitions for the WM8960_REG_CLK1 register.
 */
#define WM8960_CLK1_ADCDIV_BY_1         0x000u /**< ADC Sample rate divider (SYSCLK/(1 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_1_5       0x040u /**< ADC Sample rate divider (SYSCLK/(1.5 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_2         0x080u /**< ADC Sample rate divider (SYSCLK/(2 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_3         0x0C0u /**< ADC Sample rate divider (SYSCLK/(3 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_4         0x100u /**< ADC Sample rate divider (SYSCLK/(4 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_5_5       0x140u /**< ADC Sample rate divider (SYSCLK/(5.5 * 256)) */
#define WM8960_CLK1_ADCDIV_BY_6         0x180u /**< ADC Sample rate divider (SYSCLK/(6 * 256)) */

#define WM8960_CLK1_DACDIV_BY_1         0x000u /**< DAC Sample rate divider (SYSCLK/(1 * 256)) */
#define WM8960_CLK1_DACDIV_BY_1_5       0x008u /**< DAC Sample rate divider (SYSCLK/(1.5 * 256)) */
#define WM8960_CLK1_DACDIV_BY_2         0x010u /**< DAC Sample rate divider (SYSCLK/(2 * 256)) */
#define WM8960_CLK1_DACDIV_BY_3         0x018u /**< DAC Sample rate divider (SYSCLK/(3 * 256)) */
#define WM8960_CLK1_DACDIV_BY_4         0x020u /**< DAC Sample rate divider (SYSCLK/(4 * 256)) */
#define WM8960_CLK1_DACDIV_BY_5_5       0x028u /**< DAC Sample rate divider (SYSCLK/(5.5 * 256)) */
#define WM8960_CLK1_DACDIV_BY_6         0x030u /**< DAC Sample rate divider (SYSCLK/(6 * 256)) */

#define WM8960_CLK1_SYSCLKDIV_BY_1      0x000u /**< SYSCLK Pre-divider. Divide SYSCLK by 1 */
#define WM8960_CLK1_SYSCLKDIV_BY_2      0x004u /**< SYSCLK Pre-divider. Divide SYSCLK by 2 */

#define WM8960_CLK1_CLKSEL_MCLK         0x000u /**< SYSCLK derived from MCLK */
#define WM8960_CLK1_CLKSEL_PLL          0x001u /**< SYSCLK derived from PLL output */
/** \} WM8960_REG_CLK1 */

/**
 * \{
 * @name WM8960_REG_CTR1 (0x05)
 * Bit definitions for the WM8960_REG_CTR1 register.
 */
#define WM8960_CTR1_DACDIV2_6B_EN       0x080u /**< DAC 6dB Attenuate Enable, */

#define WM8960_CTR1_ADCPOL_LR_INV       0x060u /**< ADC polarity control. ADC L and R inverted */
#define WM8960_CTR1_ADCPOL_L_INV        0x040u /**< ADC polarity control. ADC R inverted */
#define WM8960_CTR1_ADCPOL_R_INV        0x020u /**< ADC polarity control. ADC L inverted */

#define WM8960_CTR1_DACMU_NO            0x000u /**< DAC Digital Soft Mute disabled */
#define WM8960_CTR1_DACMU_MUTE          0x008u /**< DAC Digital Soft Mute enabled */

#define WM8960_CTR1_DEEMPH_48_KHZ       0x006u /**< De-emphasis Control for 48 KHz sample rate */
#define WM8960_CTR1_DEEMPH_44_1_KHZ     0x004u /**< De-emphasis Control for 44.1 KHz sample rate */
#define WM8960_CTR1_DEEMPH_32_KHZ       0x002u /**< De-emphasis Control for 32 KHz sample rate */

#define WM8960_CTR1_ADCHPD_DIS          0x001u /**< ADC High Pass Filter Disable */
/** \} WM8960_REG_CTR1 */

/**
 * \{
 * @name WM8960_REG_CTR2 (0x06)
 * Bit definitions for the WM8960_REG_CTR2 register.
 */
#define WM8960_CTR2_DACPOL_LR_INV       0x060u /**< DAC polarity control. DAC L and R inverted */
#define WM8960_CTR2_DACPOL_L_INV        0x040u /**< DAC polarity control. DAC R inverted */
#define WM8960_CTR2_DACPOL_R_INV        0x020u /**< DAC polarity control. DAC L inverted */

#define WM8960_CTR2_DACSMM_GRAD         0x008u /**< DAC Soft Mute Mode. Disabling soft-mute
                                                    (DACMU=0) will cause the volume to ramp
                                                    up gradually to the LDACVOL/RDACVOL settings */

#define WM8960_CTR2_DACMR_SLOW_RMP      0x004u /**< DAC Soft Mute Ramp Rate.
                                                    Slow ramp (1.5kHz at fs=48k, providing
                                                    maximum delay of 171ms) */

#define WM8960_CTR2_DACSLOPE_SS         0x002u /**< DAC filter characteristics. Sloping stopband */
/** \} WM8960_REG_CTR2 */

/**
 * \{
 * @name WM8960_REG_AUDIO_INTF0 (0x07)
 * Bit definitions for the WM8960_REG_AUDIO_INTF0 register.
 */
#define WM8960_AUDIO_INTF0_ALRSWAP          0x100u /**< Left/Right ADC Channel Swap. Swap left
                                                        and right ADC data in audio interface */

#define WM8960_AUDIO_INTF0_BCLKINV          0x080u /**< BCLK invert bit. BCLK inverted */

#define WM8960_AUDIO_INTF0_MS_MASTER        0x040u /**< Master Mode. */
#define WM8960_AUDIO_INTF0_MS_SLAVE         0x000u /**< Slave Mode. */

#define WM8960_AUDIO_INTF0_DLRSWAP          0x020u /**< Left/Right DAC Channel Swap. Swap left
                                                        and right DAC data in audio interface */

#define WM8960_AUDIO_INTF0_LRP_INV          0x010u /**< Right, left and I2S modes – LRCLK polarity.
                                                        Invert LRCLK polarity*/

#define WM8960_AUDIO_INTF0_WL_16BITS        0x000u /**< Audio Data Word Length. 16 bits */
#define WM8960_AUDIO_INTF0_WL_20BITS        0x004u /**< Audio Data Word Length. 20 bits */
#define WM8960_AUDIO_INTF0_WL_24BITS        0x008u /**< Audio Data Word Length. 24 bits */
#define WM8960_AUDIO_INTF0_WL_32BITS        0x00Cu /**< Audio Data Word Length. 32 bits */

#define WM8960_AUDIO_INTF0_FORMAT_R_JUSTIF  0x000u /**< Right justified format */
#define WM8960_AUDIO_INTF0_FORMAT_L_JUSTIF  0x001u /**< Left justified format */
#define WM8960_AUDIO_INTF0_FORMAT_I2S_MODE  0x002u /**< I2S format */
#define WM8960_AUDIO_INTF0_FORMAT_DSP_MODE  0x003u /**< DSP Mode */
/** \} WM8960_REG_AUDIO_INTF0 */

/**
 * \{
 * @name WM8960_REG_CLK2 (0x08)
 * Bit definitions for the WM8960_REG_CLK2 register.
 */
#define WM8960_CLK2_DCLKDIV_BY_2        0x040u /**< Class D switching clock divider (SYSCLK/2) */
#define WM8960_CLK2_DCLKDIV_BY_3        0x080u /**< Class D switching clock divider (SYSCLK/3) */
#define WM8960_CLK2_DCLKDIV_BY_4        0x0C0u /**< Class D switching clock divider (SYSCLK/4) */
#define WM8960_CLK2_DCLKDIV_BY_6        0x100u /**< Class D switching clock divider (SYSCLK/6) */
#define WM8960_CLK2_DCLKDIV_BY_8        0x140u /**< Class D switching clock divider (SYSCLK/8) */
#define WM8960_CLK2_DCLKDIV_BY_12       0x180u /**< Class D switching clock divider (SYSCLK/12) */
#define WM8960_CLK2_DCLKDIV_BY_16       0x1C0u /**< Class D switching clock divider (SYSCLK/16) */

#define WM8960_CLK2_BCLKDIV_BY_1_5      0x001u /**< BCLK Frequency (SYSCLK/1.5) */
#define WM8960_CLK2_BCLKDIV_BY_2        0x002u /**< BCLK Frequency (SYSCLK/2) */
#define WM8960_CLK2_BCLKDIV_BY_3        0x003u /**< BCLK Frequency (SYSCLK/3) */
#define WM8960_CLK2_BCLKDIV_BY_4        0x004u /**< BCLK Frequency (SYSCLK/4) */
#define WM8960_CLK2_BCLKDIV_BY_5_5      0x005u /**< BCLK Frequency (SYSCLK/5.5) */
#define WM8960_CLK2_BCLKDIV_BY_6        0x006u /**< BCLK Frequency (SYSCLK/6) */
#define WM8960_CLK2_BCLKDIV_BY_8        0x007u /**< BCLK Frequency (SYSCLK/8) */
#define WM8960_CLK2_BCLKDIV_BY_11       0x008u /**< BCLK Frequency (SYSCLK/11) */
#define WM8960_CLK2_BCLKDIV_BY_12       0x009u /**< BCLK Frequency (SYSCLK/12) */
#define WM8960_CLK2_BCLKDIV_BY_16       0x00Au /**< BCLK Frequency (SYSCLK/16) */
#define WM8960_CLK2_BCLKDIV_BY_22       0x00Bu /**< BCLK Frequency (SYSCLK/22) */
#define WM8960_CLK2_BCLKDIV_BY_24       0x00Cu /**< BCLK Frequency (SYSCLK/24) */
#define WM8960_CLK2_BCLKDIV_BY_32       0x00Du /**< BCLK Frequency (SYSCLK/32) */
/** \} WM8960_REG_CLK2 */

/**
 * \{
 * @name WM8960_REG_AUDIO_INTF1 (0x09)
 * Bit definitions for the WM8960_REG_AUDIO_INTF1 register.
 */
#define WM8960_AUDIO_INTF1_ALRCGPIO_GPIO    0x040u /**< GPIO1 Pin Function Select */

#define WM8960_AUDIO_INTF1_WL8_8BIT         0x020u /**< 8-Bit Word Length Select.
                                                      Device operates in 8-bit mode. */

#define WM8960_AUDIO_INTF1_DACCOMP_ALAW     0x010u /**< DAC companding. A-Law */
#define WM8960_AUDIO_INTF1_DACCOMP_ULAW     0x018u /**< DAC companding. u-Law */

#define WM8960_AUDIO_INTF1_ADCCOMP_ULAW     0x004u /**< ADC companding. u-Law */
#define WM8960_AUDIO_INTF1_ADCCOMP_ALAW     0x006u /**< ADC companding. A-Law */

#define WM8960_AUDIO_INTF1_LOOPBACK_EN      0x001u /**< Loopback enabled, ADC data
                                                      output is fed directly into
                                                      DAC data input. */
/** \} WM8960_REG_AUDIO_INTF1 */

/**
 * \{
 * @name WM8960_REG_LEFT_DAC_VOL (0x0A) and WM8960_REG_RIGHT_DAC_VOL (0x0B)
 * Bit definitions for the WM8960_REG_LEFT_DAC_VOL and
 * WM8960_REG_RIGHT_DAC_VOL register.
 */
#define WM8960_LEFT_RIGHT_DAC_VOL_DACVU_UP  0x100u /**< DAC Volume Update bit */

#define WM8960_LEFT_RIGHT_DAC_VOL_0dB       0x0FFu /**< DAC Volume Control 0dB  */
#define WM8960_LEFT_RIGHT_DAC_VOL_127dB     0x001u /**< DAC Volume Control -127dB */
/** \} WM8960_REG_LEFT_DAC_VOL and WM8960_REG_RIGHT_DAC_VOL */

/**
 * \{
 * @name WM8960_REG_3D_CTR (0x10)
 * Bit definitions for the WM8960_REG_3D_CTR register.
 */
#define WM8960_3D_CTR_3DUC_LOW              0x040u /**< 3D Enhance Filter Upper
                                                        Cut-Off Frequency Low (Recommended for
                                                        fs<32kHz) */
#define WM8960_3D_CTR_3DUC_HIGH             0x000u /**< 3D Enhance Filter Upper
                                                        Cut-Off Frequency High (Recommended for
                                                        fs>=32kHz)*/
#define WM8960_3D_CTR_3DLC_LOW              0x000u /**< 3D Enhance Filter Lower
                                                        Cut-Off Frequency Low (Recommended for
                                                        fs>=32kHz) */
#define WM8960_3D_CTR_3DLC_HIGH             0x020u /**< 3D Enhance Filter Lower
                                                        Cut-Off Frequency High (Recommended for
                                                        fs<32kHz) */

#define WM8960_3D_CTR_3DDEPTH_100           0x01Eu /**< 3D Stereo Depth 100% */

#define WM8960_3D_CTR_3DEN                  0x001u /**< 3D Stereo Enhancement Enable */
/** \} WM8960_REG_3D_CTR */

/**
 * \{
 * @name WM8960_REG_LEFT_ADC_VOL (0x15) and WM8960_REG_RIGHT_ADC_VOL (0x16)
 * Bit definitions for the WM8960_REG_LEFT_ADC_VOL and
 * WM8960_REG_LEFT_ADC_VOL register
 */
#define WM8960_LEFT_RIGHT_ADC_VOL_ADCVU_UP         0x100u /**< ADC Volume Update bit */

#define WM8960_LEFT_RIGHT_ADC_VOL_ADCVOL_0dB       0x0C3u /**< ADC Volume Control 0dB  */
#define WM8960_LEFT_RIGHT_ADC_VOL_ADCVOL_30dB      0x0FFu /**< DAC Volume Control 30dB */
/** \} WM8960_REG_LEFT_ADC_VOL and WM8960_REG_LEFT_ADC_VOL */

/**
 * \{
 * @name WM8960_REG_PWR_MGMT1 (0x19)
 * Bit definitions for the WM8960_REG_PWR_MGMT1 register
 */
/* WM8960_REG_PWR_MGMT1 (0x19) bits */
#define WM8960_PWR_MGMT1_VMIDSEL_50K      0x080u /**< 2 x 50k Divider enabled
                                                      (for playback/record) */
#define WM8960_PWR_MGMT1_VMIDSEL_250K     0x100u /**< 2 x 250k Divider enabled
                                                      (for low-power standby) */
#define WM8960_PWR_MGMT1_VMIDSEL_5K       0x180u /**< 2 x 5k Divider enabled
                                                      (for fast start-up) */

#define WM8960_PWR_MGMT1_VREF_UP          0x040u /**< VREF power up */
#define WM8960_PWR_MGMT1_AINL_UP          0x020u /**< Analogue in PGA Left power up */
#define WM8960_PWR_MGMT1_AINR_UP          0x010u /**< Analogue in PGA Right power up */
#define WM8960_PWR_MGMT1_ADCL_UP          0x008u /**< ADC Left power up */
#define WM8960_PWR_MGMT1_ADCR_UP          0x004u /**< ADC Right power up */
#define WM8960_PWR_MGMT1_MICB_UP          0x002u /**< MICBIAS power up*/
#define WM8960_PWR_MGMT1_DIGENB_DI        0x001u /**< Master Clock disabled */
/** \} WM8960_REG_PWR_MGMT1 */

/**
 * \{
 * @name WM8960_REG_PWR_MGMT2 (0x1A)
 * Bit definitions for the WM8960_REG_PWR_MGMT2 register
 */
#define WM8960_PWR_MGMT2_DACL_UP          0x100u /**< DAC Left power up */
#define WM8960_PWR_MGMT2_DACR_UP          0x080u /**< DAC Right power up */
#define WM8960_PWR_MGMT2_LOUT1_UP         0x040u /**< LOUT1 Output Buffer power up */
#define WM8960_PWR_MGMT2_ROUT1_UP         0x020u /**< ROUT1 Output Buffer power up */
#define WM8960_PWR_MGMT2_SPKL_UP          0x010u /**< SPK_LP/SPK_LN Output Buffers power up */
#define WM8960_PWR_MGMT2_SPKR_UP          0x008u /**< SPK_RP/SPK_RN Output Buffers power up */
#define WM8960_PWR_MGMT2_OUT3_UP          0x002u /**< OUT3 Output Buffer power up */
#define WM8960_PWR_MGMT2_PLL_EN_UP        0x001u /**< PLL power up */
/** \} WM8960_REG_PWR_MGMT2 */

/**
 * \{
 * @name WM8960_REG_ADCL_SIG_PTH (0x20) and WM8960_REG_ADCR_SIG_PTH (0x21)
 * Bit definitions for the WM8960_REG_ADCL_SIG_PTH and
 * WM8960_REG_ADCR_SIG_PTH register
 */
#define WM8960_ADCL_ADCR_SIG_PTH_MN1_CON          0x100u /**< Connect (L/R)INPUT1 to inverting input
                                                              of Input PGA */
#define WM8960_ADCL_ADCR_SIG_PTH_MP3_CON          0x080u /**< Connect (L/R)INPUT3 to non-inverting
                                                              input of Input PGA */
#define WM8960_ADCL_ADCR_SIG_PTH_MP2_CON          0x040u /**< Connect (L/R)INPUT2 to non-inverting
                                                              input of Input PGA */

#define WM8960_ADCL_ADCR_SIG_PTH_MICBOOST_0dB     0x000u /**< Input PGA Boost Gain 0dB */
#define WM8960_ADCL_ADCR_SIG_PTH_MICBOOST_13dB    0x010u /**< Input PGA Boost Gain 13dB */
#define WM8960_ADCL_ADCR_SIG_PTH_MICBOOST_20dB    0x020u /**< Input PGA Boost Gain 20dB */
#define WM8960_ADCL_ADCR_SIG_PTH_MICBOOST_29dB    0x030u /**< Input PGA Boost Gain 29dB */

#define WM8960_ADCL_ADCR_SIG_PTH_MIC2B_CON        0x008u /**< Connect I/P PGA to I/P Boost Mixer */
/** \} WM8960_REG_ADCL_SIG_PTH and WM8960_REG_ADCR_SIG_PTH */

/**
 * \{
 * @name WM8960_REG_LEFT_OUT_MIX (0x22)
 * Bit definitions for the WM8960_REG_LEFT_OUT_MIX register
 */
/* WM8960_REG_LEFT_OUT_MIX (0x22) bits */ /**< */
#define WM8960_LEFT_OUT_MIX_LD2LO_EN         0x100u /**< Connect Left DAC to Left Output Mixer */
#define WM8960_LEFT_OUT_MIX_LI2LO_EN         0x080u /**< Connect LINPUT3 to Left Output Mixer */
/** \} WM8960_REG_LEFT_OUT_MIX */

/**
 * \{
 * @name WM8960_REG_RIGHT_OUT_MIX (0x25)
 * Bit definitions for the WM8960_REG_RIGHT_OUT_MIX register
 */
#define WM8960_RIGHT_OUT_MIX_RD2RO_EN         0x100u /**< Connect Right DAC to Right Output Mixer */
#define WM8960_RIGHT_OUT_MIX_RI2RO_EN         0x080u /**< Connect RINPUT3 to Right Output Mixer */
/** \} WM8960_REG_RIGHT_OUT_MIX */

/**
 * \{
 * @name WM8960_REG_MONO_OUT_MIX1 (0x26)
 *   and WM8960_REG_MONO_OUT_MIX2 (0x27)
 *
 * Bit definitions for the WM8960_REG_MONO_OUT_MIX1
 * and WM8960_REG_MONO_OUT_MIX2 register.
 */
#define WM8960_MONO_OUT_MIX1_MIX2_2MO_DI           0x000u /**< Disconnect Left Output Mixer to
                                                               Mono Output Mixer Control */
#define WM8960_MONO_OUT_MIX1_MIX2_2MO_EN           0x080u /**< Connect Right Output Mixer to
                                                               Mono Output Mixer Control */
/** \} WM8960_REG_MONO_OUT_MIX1 and WM8960_REG_MONO_OUT_MIX1 */

/**
 * \{
 * @name WM8960_REG_LOUT2_VOL (0x28)
 *   and WM8960_REG_ROUT2_VOL (0x29)
 *
 * Bit definitions for the WM8960_REG_LOUT2_VOL
 * and WM8960_REG_ROUT2_VOL register
 */
#define WM8960_LOUT2_ROUT2_VOL_SPKVU            0x100u /**< Speaker Volume Update */
#define WM8960_LOUT2_ROUT2_VOL_SPKRZC           0x080u /**< Speaker Zero Cross Enable */

#define WM8960_LOUT2_ROUT2_VOL_SPKRVOL_0dB      0x079u /**< SPK_RP/SPK_RN Volume 0dB */
#define WM8960_LOUT2_ROUT2_VOL_SPKRVOL_6dB      0x07Fu /**< SPK_RP/SPK_RN Volume 6dB */

/** \} WM8960_REG_LOUT2_VOL and WM8960_REG_ROUT2_VOL */

/**
 * \{
 * @name WM8960_REG_PWR_MGMT3 (0x2F)
 * Bit definitions for the WM8960_REG_PWR_MGMT3 register
 */
#define WM8960_PWR_MGMT3_LMIC_UP          0x020u /**< Left Channel Input PGA Enable */
#define WM8960_PWR_MGMT3_RMIC_UP          0x010u /**< Right Channel Input PGA Enable */
#define WM8960_PWR_MGMT3_LOMIX_UP         0x008u /**< Left Output Mixer Enable */
#define WM8960_PWR_MGMT3_ROMIX_UP         0x004u /**< Right Output Mixer Enable */
/** \} WM8960_REG_PWR_MGMT3 */

/**
 * \{
 * @name WM8960_REG_PLL_N (0x34)
 * Bit definitions for the WM8960_REG_PWR_MGMT3 register
 */
#define WM8960_PLL_N_PLLPRESCALE_DI          0x000u /**< MCLK input directly into PLL */
#define WM8960_PLL_N_PLLPRESCALE_EN          0x010u /**< Divide MCLK by 2 before input to PLL */

#define WM8960_PLL_N_SDM_INT                 0x000u /**< Enable Integer Mode */
#define WM8960_PLL_N_SDM_FRAC                0x020u /**< Enable Fractional Mode */

#define WM8960_PLL_N_OPCLKDIV_BY_1           0x000u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK) */
#define WM8960_PLL_N_OPCLKDIV_BY_2           0x040u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK/2) */
#define WM8960_PLL_N_OPCLKDIV_BY_3           0x080u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK/3) */
#define WM8960_PLL_N_OPCLKDIV_BY_4           0x0C0u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK/4) */
#define WM8960_PLL_N_OPCLKDIV_BY_5_5         0x100u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK/5.5) */
#define WM8960_PLL_N_OPCLKDIV_BY_6           0x140u /**< SYSCLK Output to GPIO Clock Division ratio.
                                                       (SYSCLK/6) */
/** \} WM8960_REG_PLL_N */

/** Invalid argument was passed into a function. */
#define MTB_RSLT_WM8960_BAD_ARG false

/**
 * Enumeration for the WM8960 I2C Interface Register Addresses
 *
 * The entries below are intended for use with these functions:
 * - mtb_wm8960_read()
 * - mtb_wm8960_write()
 * - mtb_wm8960_set()
 * - mtb_wm8960_clear()
 */
typedef enum
{
    WM8960_REG_LEFT_IN_VOL,             /**< 0x00: Left Input volume */
    WM8960_REG_RIGHT_IN_VOL,            /**< 0x01: Right Input volume */
    WM8960_REG_LOUT1_VOL,               /**< 0x02: LOUT1 volume */
    WM8960_REG_ROUT1_VOL,               /**< 0x03: ROUT1 volume */
    WM8960_REG_CLK1,                    /**< 0x04: Clocking (1) */
    WM8960_REG_CTR1,                    /**< 0x05: ADC & DAC Control (CTR1) */
    WM8960_REG_CTR2,                    /**< 0x06: ADC & DAC Control (CTR2) */
    WM8960_REG_AUDIO_INTF0,             /**< 0x07: Audio Interface (0) */
    WM8960_REG_CLK2,                    /**< 0x08: Clocking (2) */
    WM8960_REG_AUDIO_INTF1,             /**< 0x09: Audio Interface (1) */
    WM8960_REG_LEFT_DAC_VOL,            /**< 0x0A: Left DAC volume */
    WM8960_REG_RIGHT_DAC_VOL,           /**< 0x0B: Right DAC volume */
    WM8960_REG_RSVD0,                   /*   0x0C: Reserved */
    WM8960_REG_RSVD1,                   /*   0x0D: Reserved */
    WM8960_REG_RSVD2,                   /*   0x0E: Reserved */
    WM8960_REG_RESET,                   /**< 0x0F: Reset */
    WM8960_REG_3D_CTR,                  /**< 0x10: 3D Control */
    WM8960_REG_ALC1,                    /**< 0x11: ALC1 */
    WM8960_REG_ALC2,                    /**< 0x12: ALC2 */
    WM8960_REG_ALC3,                    /**< 0x13: ALC3 */
    WM8960_REG_NOISE_GATE,              /**< 0x14: Noise Gate */
    WM8960_REG_LEFT_ADC_VOL,            /**< 0x15: Left ADC volume */
    WM8960_REG_RIGHT_ADC_VOL,           /**< 0x16: Right ADC volume */
    WM8960_REG_ADD_CTL1,                /**< 0x17: Additional Control (1) */
    WM8960_REG_ADD_CTL2,                /**< 0x18: Additional Control (2) */
    WM8960_REG_PWR_MGMT1,               /**< 0x19: Power Management (1) */
    WM8960_REG_PWR_MGMT2,               /**< 0x1A: Power Management (2) */
    WM8960_REG_ADD_CTL3,                /**< 0x1B: Additional Control (3) */
    WM8960_REG_ANTI_POP1,               /**< 0x1C: Anti-pop 1 */
    WM8960_REG_ANTI_POP2,               /**< 0x1D: Anti-pop 2 */
    WM8960_REG_RSVD3,                   /*   0x1E: Reserved */
    WM8960_REG_RSVD4,                   /*   0x1F: Reserved */
    WM8960_REG_ADCL_SIG_PTH,            /**< 0x20: ADCL signal path */
    WM8960_REG_ADCR_SIG_PTH,            /**< 0x21: ADCR signal path */
    WM8960_REG_LEFT_OUT_MIX,            /**< 0x22: Left out Mix (1) */
    WM8960_REG_RSVD5,                   /*   0x23: Reserved */
    WM8960_REG_RSVD6,                   /*   0x24: Reserved */
    WM8960_REG_RIGHT_OUT_MIX,           /**< 0x25: Right out Mix (2) */
    WM8960_REG_MONO_OUT_MIX1,           /**< 0x26: Mono out Mix (1) */
    WM8960_REG_MONO_OUT_MIX2,           /**< 0x27: Mono out Mix (2) */
    WM8960_REG_LOUT2_VOL,               /**< 0x28: LOUT2 volume */
    WM8960_REG_ROUT2_VOL,               /**< 0x29: ROUT2 volume */
    WM8960_REG_MONO_OUT_VOL,            /**< 0x2A: Mono out volume */
    WM8960_REG_IN_BOOST_MIX1,           /**< 0x2B: Input boost mixer (1) */
    WM8960_REG_IN_BOOST_MIX2,           /**< 0x2C: Input boost mixer (2) */
    WM8960_REG_BYPASS1,                 /**< 0x2D: Bypass (1) */
    WM8960_REG_BYPASS2,                 /**< 0x2E: Bypass (2) */
    WM8960_REG_PWR_MGMT3,               /**< 0x2F: Power Management (3) */
    WM8960_REG_ADD_CTL4,                /**< 0x30: Additional Control (4) */
    WM8960_REG_CLASS_D_CTL1,            /**< 0x31: Class D Control (1) */
    WM8960_REG_RSVD7,                   /*   0x32: Reserved */
    WM8960_REG_CLASS_D_CTL3,            /**< 0x33: Class D Control (3) */
    WM8960_REG_PLL_N,                   /**< 0x34: PLL N */
    WM8960_REG_PLL_K1,                  /**< 0x35: PLL K 1 */
    WM8960_REG_PLL_K2,                  /**< 0x36: PLL K 2 */
    WM8960_REG_PLL_K3                   /**< 0x37: PLL K 3 */
} mtb_wm8960_reg_t;

/**
 * Enumeration of bit flags enabling/disabling features of the WM8960
 * audio codec.
 *
 * \note The values of the enumeration can be OR'ed to enable multiple
 * features
 */
typedef enum
{
    WM8960_FEATURE_NONE        = 0x00,                   /**< No features enabled */
    WM8960_FEATURE_MICROPHONE1 = 0x01,                   /**< Enable the microphone */
    WM8960_FEATURE_MICROPHONE2 = 0x02,                   /**< Enable the microphone */
    WM8960_FEATURE_MICROPHONE3 = 0x04,                   /**< Enable the microphone */
    WM8960_FEATURE_HEADPHONE   = 0x08,                    /**< Enable the headphone */
    WM8960_FEATURE_SPEAKER     = 0x10                    /**< Enable the class D speaker */
} mtb_wm8960_features_t;

#define WM8960_FEATURE_MICROPHONES (WM8960_FEATURE_MICROPHONE1|WM8960_FEATURE_MICROPHONE2|WM8960_FEATURE_MICROPHONE3)

/**
 * Enumeration for supported sample rates for ADC and DAC in the WM8960
 * audio codec.
 */
typedef enum
{
    WM8960_ADC_DAC_SAMPLE_RATE_48_KHZ,                  /**< 48 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_44_1_KHZ,                /**< 44.1 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_32_KHZ,                  /**< 32 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_24_KHZ,                  /**< 24 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_22_05_KHZ,               /**< 22.05 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_16_KHZ,                  /**< 16 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_12_KHZ,                  /**< 12 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_11_025_KHZ,              /**< 11.025 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_8_018_KHZ,               /**< 8.018 KHz sample rate */
    WM8960_ADC_DAC_SAMPLE_RATE_8_KHZ                    /**< 8 KHz sample rate */
} mtb_wm8960_adc_dac_sample_rate_t;

/**
 * Enumeration for supported word lengths in the WM8960
 * audio codec.
 */
typedef enum
{
    WM8960_WL_16BITS = WM8960_AUDIO_INTF0_WL_16BITS,    /**< 16-bit word length */
    WM8960_WL_20BITS = WM8960_AUDIO_INTF0_WL_20BITS,    /**< 20-bit word length */
    WM8960_WL_24BITS = WM8960_AUDIO_INTF0_WL_24BITS,    /**< 24-bit word length */
    WM8960_WL_32BITS = WM8960_AUDIO_INTF0_WL_32BITS     /**< 32-bit word length */
} mtb_wm8960_word_length_t;

/**
 * Enumeration to select mode of operation of the WM8960 audio codec.
 */
typedef enum
{
    WM8960_MODE_MASTER = WM8960_AUDIO_INTF0_MS_MASTER,  /**< Master mode */
    WM8960_MODE_SLAVE  = WM8960_AUDIO_INTF0_MS_SLAVE    /**< Slave mode */
} mtb_wm8960_mode_t;


/**
 * @brief Provide an alternative initialized Wire object. If this is not called we
 * automatically use the Wire object and initialize it with begin().
 * 
 * @ingroup wm8960
 * @param i2c_inst 
 * @return true 
 * @return false 
 */
bool mtb_wm8960_set_wire(void* i2c_inst);


/**
 * @brief Defines the number of times we retry to update a register value via I2C.
 * This might help if you have an instable I2C connection. By default we set it to 1
 * which will cause the initialization to fail when the register update fails.
 * @ingroup wm8960
 * @param count; 0: retry endlessly until success; any number > 0 = number of retries 
 * 
*/
void mtb_wm8960_set_write_retry_count(uint32_t count);


/**
 * @brief Initialize the I2C communication with the audio codec, reset the codec
 * and apply default configuration based on the feature(s) requested.
 *
 * If either WM8960_FEATURE_MICROPHONE or WM8960_FEATURE_HEADPHONE is requested,
 * the following operations will be performed,
 * For either feature features,
 * - Set VMID=50K and Enable VREF
 *
 * For WM8960_FEATURE_MICROPHONE,
 * - Enable AINL, AINR, ADCL and ADCR
 * - Enable left (LMIC) and right channel (RMIC) input PGA
 * - LINPUT1 to PGA (LMN1), Connect left input PGA to left
 *   input boost (LMIC2B), Left PGA Boost = 0dB
 * - RINPUT1 to PGA (RMN1), Connect right input PGA to right
 *   input boost (RMIC2B), Right PGA Boost = 0dB
 * - Unmute left input PGA (LINMUTE), Left Input PGA Vol = 0dB,
 *   Volume Update
 * - Unmute right input PGA (RINMUTE), Right Input PGA Vol = 0dB,
 *   Volume Update
 * - Left ADC Vol = 0dB, Volume Update
 * - Right ADC Vol = 0dB, Volume Update
 *
 * For WM8960_FEATURE_HEADPHONE,
 * - Enable DACL, DACR, LOUT1 and ROUT1
 * - Enable left output mixer (LOMIX) and right output mixer (ROMIX)
 * - Left DAC to left output mixer enabled (LD2LO), 0dB
 * - Right DAC to right output mixer enabled (RD2RO), 0dB
 * - LOUT1 Vol = 0dB, volume update enabled
 * - ROUT1 Vol = 0dB, volume update enabled
 * - Unmute DAC digital soft mute
 *
 * @param[in] features Features to enabled during initialization. See mtb_wm8960_features_t
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_init(uint8_t features);

/**
 * @brief Frees up any resources allocated by the driver as part of \ref mtb_wm8960_init().
 * @ingroup wm8960
 */
void mtb_wm8960_free();

/**
 * @brief This function updates the volume of both the left and right channels of the
 * microphone input.
 *
 * @param[in] volume - Steps of 0.75dB, where:
 *            Minimum volume:  -17.25dB (0x00)
 *            Maximum volume:  +30dB    (0x3F)
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_adjust_input_volume(uint8_t volume);

/**
 * @brief This function updates the volume of both the left and right channels of the
 * headphone output.
 *
 * @param[in] volume - Steps of 1dB, where:
 *            Minimum volume: -73dB (0x30)
 *            Maximum volume: +6dB  (0x7F)
 *            Mute: (0x00~0x2F)
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_adjust_heaphone_output_volume(uint8_t volume);

/**
 * @brief This function updates the volume of both the left and right channels of the
 * speaker output.
 *
 * @param[in] volume - Steps of 1dB, where:
 *            Minimum volume: -73dB (0x30)
 *            Maximum volume: +6dB  (0x7F)
 *            Mute: (0x00~0x2F)
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_adjust_speaker_output_volume(uint8_t volume);


/**
 * @brief This function updates the volume of both the left and right channels of the
 * speaker and headphones.
 *
 * @param[in] volume - Steps of 1dB, where:
 *            Minimum volume: -73dB (0x30)
 *            Maximum volume: +6dB  (0x7F)
 *            Mute: (0x00~0x2F)
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_set_output_volume(uint8_t volume);


/**
 * @brief This function powers up the modules the required for the features enabled using
 * \ref mtb_wm8960_init. This function is called in conjunction with \ref mtb_wm8960_deactivate.
 *
 * \note This function only updates the power management registers
 * (R25[0x19], R26[0x1A], R47[0x2F]) to enable the modules required for the enabled features.
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_activate(void);

/**
 * @brief This function powers down the modules the required for the features enabled using
 * \ref mtb_wm8960_init.
 *
 * \note This function only updates the power management registers
 * (R25[0x19], R26[0x1A], R47[0x2F]) to disable the modules required for the enabled features.
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_deactivate(void);

/**
 * @brief This function reads value of an audio codec register.
 *
 * @param[in]  reg    The audio codec register to read
 * @param[out] data   The reference to read the audio codec register data into.
 *
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_read(mtb_wm8960_reg_t reg, uint16_t* data);

/**
 * @brief This function writes data to an audio codec register.
 *
 * @param[in] reg   The audio codec register to update
 * @param[in] data  The data to be written to the audio codec register
 *
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_write(mtb_wm8960_reg_t reg, uint16_t data);

/**
 * @brief This function sets bits in a register.  This function can be used instead
 * of mtb_wm8960_write() if you want to change a single bit or select bits in
 * the register and preserve the value of other bits in the register. Only the bits
 * set to 1 in the mask are affected.
 *
 * @param[in] reg   The audio codec register to update
 * @param[in] mask  The mask used to set bits in the register
 *
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_set(mtb_wm8960_reg_t reg, uint16_t mask);

/**
 * @brief This function clears bits in a register.  This function can be used instead
 * of mtb_wm8960_write() if you want to change a single bit or select bits in
 * the register and preserve the value of other bits in the register. Only the bits
 * set to 1 in the mask are affected.
 *
 * @param[in] reg   The audio codec register to update
 * @param[in] mask  The mask used to clear bits in the register
 *
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_clear(mtb_wm8960_reg_t reg, uint16_t mask);

/**
 * @brief This function configures the master clock and the digital interface for the audio codec.
 *
 * @param[in] mclk_hz       The master clock (MCLK) frequency
 * @param[in] enable_pll    Set true to enable PLL and false to disable PLL
 * @param[in] sample_rate   Sample rate for the ADC and DAC
 * @param[in] word_length   Word length
 * @param[in] mode          Mode the audio codec to operate as.
 *
 * @ingroup wm8960
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_configure_clocking(uint32_t mclk_hz, bool enable_pll,
                                        mtb_wm8960_adc_dac_sample_rate_t sample_rate,
                                        mtb_wm8960_word_length_t word_length,
                                        mtb_wm8960_mode_t mode);


/**
 * @brief This function dumps the actual register values
 *
 * @param[in] reg   The audio codec register to update
 * @param[in] mask  The mask used to clear bits in the register
 * @ingroup wm8960
 *
 * @return true if properly initialized, else an error indicating what went wrong.
 */
bool mtb_wm8960_dump();


/**
 * Platform dependent i2c write. If you compile this library outside of Arduino you need to provide
 * your own implementation.
*/
bool i2c_write(uint8_t address, uint8_t data[2]);


#if defined(__cplusplus)
}
#endif

/** \} group_board_libs */
