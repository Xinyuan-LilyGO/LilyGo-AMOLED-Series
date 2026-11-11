// code from https://github.com/donny681/esp-adf/blob/master/components/audio_driver/AC101/AC101.c
// documentation see https://usermanual.wiki/Document/xpowers20AC10120User20Manual20v11.447217432/html#pf25


#include <string.h>
#include "ac101.h"

static i2c_bus_handle_t i2c_handle = NULL;
static int i2c_addr_ac101 = AC101_ADDR;

#ifndef ARDUINO 
#  define delay(n)
#endif

#define AC_ASSERT(a, format, b, ...)          \
	if ((a) != 0)                             \
	{                                         \
		AD_LOGE("AC101", format, ##__VA_ARGS__); \
		return b;                             \
	}


static error_t ac101_write_reg(uint8_t reg_add, uint16_t data){
	uint8_t send_buff[2];
	send_buff[0] = (data >> 8) & 0xff;
	send_buff[1] = data & 0xff;
    return i2c_bus_write_bytes(i2c_handle, i2c_addr_ac101, &reg_add, sizeof(reg_add), (uint8_t*) send_buff, sizeof(send_buff));
}

static error_t ac101_read_i2c(uint8_t devAddr, uint8_t reg_add, uint8_t *p_data, size_t size) {
    return i2c_bus_read_bytes(i2c_handle, devAddr, &reg_add, sizeof(reg_add), p_data, size);
}


static uint16_t ac101_read_reg(uint8_t reg_addr)
{
	uint16_t val = 0;
	uint8_t data_rd[2];
	ac101_read_i2c(i2c_addr_ac101, reg_addr, data_rd, 2);
	val = (data_rd[0] << 8) + data_rd[1];
	return val;
}


void set_codec_clk(samplerate_t sampledata)
{
	uint16_t sample_fre;
	switch (sampledata)
	{
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
		sample_fre = 44100;
	}
	ac101_write_reg(I2S_SR_CTRL, sample_fre);
}

/// Determines the value for the SRC register based on the selected input_device
uint16_t get_src_value(input_device_t input_device){
	uint16_t src_value = 0;
	switch(input_device){
		// microphone
		case ADC_INPUT_LINE1:
			src_value = 0x2020;
			break;
		// linein
		case ADC_INPUT_LINE2:
			src_value = 0x0408;
			break;
		// right mic in & left line in
		case ADC_INPUT_LINE3:
			src_value = 0x0420;
			break;
		// both
		case ADC_INPUT_ALL:
		default:
			src_value = 0x0408 | 0x2020;
			break;
	}
	return src_value;
}

error_t ac101_init(codec_config_t *codec_cfg, i2c_bus_handle_t handle, int addr)
{
	error_t res = RESULT_OK;
    i2c_handle = handle;
	if (addr>0){
		i2c_addr_ac101 = addr;
	}

	res = ac101_write_reg(CHIP_AUDIO_RS, 0x123);
	delay(1000);
	if (res != RESULT_OK)
	{
		AD_LOGE("reset failed!");
		return res;
	} else {
		AD_LOGI("reset");
	}
	res |= ac101_write_reg(SPKOUT_CTRL, 0xe880);

	//Enable the PLL from 256*44.1KHz MCLK source
	res |= ac101_write_reg(PLL_CTRL1, 0x014f);
	//res |= ac101_write_reg(PLL_CTRL2, 0x83c0);
	res |= ac101_write_reg(PLL_CTRL2, 0x8600);

	//Clocking system
	res |= ac101_write_reg(SYSCLK_CTRL, 0x8b08);
	res |= ac101_write_reg(MOD_CLK_ENA, 0x800c);
	res |= ac101_write_reg(MOD_RST_CTRL, 0x800c);
	res |= ac101_write_reg(I2S_SR_CTRL, 0x7000); //sample rate
	//AIF config
	res |= ac101_write_reg(I2S1LCK_CTRL, 0x8850);	//BCLK/LRCK
	res |= ac101_write_reg(I2S1_SDOUT_CTRL, 0xc000); //
	res |= ac101_write_reg(I2S1_SDIN_CTRL, 0xc000);
	res |= ac101_write_reg(I2S1_MXR_SRC, 0x2200); //

	res |= ac101_write_reg(ADC_SRCBST_CTRL, 0xccc4);

	res |= ac101_write_reg(ADC_SRC, get_src_value(codec_cfg->input_device));
	res |= ac101_write_reg(ADC_DIG_CTRL, 0x8000);
	res |= ac101_write_reg(ADC_APC_CTRL, 0xbbc3);

	//Path Configuration
	res |= ac101_write_reg(DAC_MXR_SRC, 0xcc00);
	res |= ac101_write_reg(DAC_DIG_CTRL, 0x8000);
	res |= ac101_write_reg(OMIXER_SR, 0x0081);
	res |= ac101_write_reg(OMIXER_DACA_CTRL, 0xf080); //}

	//* Enable Speaker output
	res |= ac101_write_reg(0x58, 0xeabd);

	AD_LOGI("init done");
	return res;
}

int ac101_get_spk_volume(void)
{
	int res;
	res = ac101_read_reg(SPKOUT_CTRL);
	res &= 0x1f;
	return res * 2;
}

error_t ac101_set_spk_volume(uint8_t volume)
{
	if (volume > 0x3f)
		volume = 0x3f;
	volume = volume / 2;

	uint16_t res;
	error_t ret;

	res = ac101_read_reg(SPKOUT_CTRL);
	res &= (~0x1f);
	volume &= 0x1f;
	res |= volume;
	ret = ac101_write_reg(SPKOUT_CTRL, res);
	return ret;
}

int ac101_get_earph_volume(void)
{
	int res;
	res = ac101_read_reg(HPOUT_CTRL);
	return (res >> 4) & 0x3f;
}

error_t ac101_set_earph_volume(uint8_t volume)
{
	if (volume > 0x3f)
		volume = 0x3f;

	uint16_t res, tmp;
	error_t ret;
	res = ac101_read_reg(HPOUT_CTRL);
	tmp = ~(0x3f << 4);
	res &= tmp;
	volume &= 0x3f;
	res |= (volume << 4);
	ret = ac101_write_reg(HPOUT_CTRL, res);
	return ret;
}

error_t ac101_set_output_mixer_gain(ac_output_mixer_gain_t gain, ac_output_mixer_source_t source)
{
	uint16_t regval, temp, clrbit;
	error_t ret;
	regval = ac101_read_reg(OMIXER_BST1_CTRL);
	switch (source)
	{
	case SRC_MIC1:
		temp = (gain & 0x7) << 6;
		clrbit = ~(0x7 << 6);
		break;
	case SRC_MIC2:
		temp = (gain & 0x7) << 3;
		clrbit = ~(0x7 << 3);
		break;
	case SRC_LINEIN:
		temp = (gain & 0x7);
		clrbit = ~0x7;
		break;
	default:
		return -1;
	}
	regval &= clrbit;
	regval |= temp;
	ret = ac101_write_reg(OMIXER_BST1_CTRL, regval);
	return ret;
}

error_t AC101_start(ac_module_t mode)
{

	error_t res = 0;
	if (mode == AC_MODULE_LINE)
	{
		res |= ac101_write_reg(0x51, 0x0408);
		res |= ac101_write_reg(0x40, 0x8000);
		res |= ac101_write_reg(0x50, 0x3bc0);
	}
	if (mode == AC_MODULE_ADC || mode == AC_MODULE_ADC_DAC || mode == AC_MODULE_LINE)
	{
		//I2S1_SDOUT_CTRL
		//res |= ac101_write_reg(PLL_CTRL2, 0x8120);
		res |= ac101_write_reg(0x04, 0x800c);
		res |= ac101_write_reg(0x05, 0x800c);
		//res |= ac101_write_reg(0x06, 0x3000);
	}
	if (mode == AC_MODULE_DAC || mode == AC_MODULE_ADC_DAC || mode == AC_MODULE_LINE)
	{
		//* Enable Headphoe output
		res |= ac101_write_reg(OMIXER_DACA_CTRL, 0xff80);
		res |= ac101_write_reg(HPOUT_CTRL, 0xc3c1);
		res |= ac101_write_reg(HPOUT_CTRL, 0xcb00);
		delay(100);
		res |= ac101_write_reg(HPOUT_CTRL, 0xfbc0);

		//* Enable Speaker output
		res |= ac101_write_reg(SPKOUT_CTRL, 0xeabd);
		delay(10);
		ac101_set_voice_volume(30);
	}

	return res;
}

error_t AC101_stop(ac_module_t mode)
{
	error_t res = 0;
	res |= ac101_write_reg(HPOUT_CTRL, 0x01);	//disable earphone
	res |= ac101_write_reg(SPKOUT_CTRL, 0xe880); //disable speaker
	return res;
}

error_t ac101_deinit(void)
{

	return ac101_write_reg(CHIP_AUDIO_RS, 0x123); //soft reset
}

error_t ac101_ctrl_state_active(codec_mode_t mode, bool ctrl_state_active)
{
	int res = 0;
	int es_mode_t = 0;

	switch (mode)
	{
	case CODEC_MODE_ENCODE:
		es_mode_t = AC_MODULE_ADC;
		break;
	case CODEC_MODE_LINE_IN:
		es_mode_t = AC_MODULE_LINE;
		break;
	case CODEC_MODE_DECODE:
		es_mode_t = AC_MODULE_DAC;
		break;
	case CODEC_MODE_BOTH:
		es_mode_t = AC_MODULE_ADC_DAC;
		break;
	default:
		es_mode_t = AC_MODULE_DAC;
		AD_LOGW("Codec mode not support, default is decode mode");
		break;
	}
	if (! ctrl_state_active)
	{
		res = AC101_stop(es_mode_t);
	}
	else
	{
		res = AC101_start(es_mode_t);
	}
	return res;
}

error_t ac101_config_i2s(codec_mode_t mode, I2SDefinition *iface)
{
	error_t res = 0;
	int bits = 0;
	int fmat = 0;
	int sample_fre = 0;
	uint16_t regval;
	switch (iface->bits) //0x10
	{
	// case BIT_LENGTH_8BITS:
	// 	bits = BIT_LENGTH_8_BITS;
	// 	break;
	case BIT_LENGTH_16BITS:
		bits = BIT_LENGTH_16_BITS;
		break;
	case BIT_LENGTH_24BITS:
		bits = BIT_LENGTH_24_BITS;
		break;
	default:
		bits = BIT_LENGTH_16_BITS;
	}

	switch (iface->fmt) //0x10
	{
	case I2S_NORMAL:
		fmat = 0x0;
		break;
	case I2S_LEFT:
		fmat = 0x01;
		break;
	case I2S_RIGHT:
		fmat = 0x02;
		break;
	case I2S_DSP:
		fmat = 0x03;
		break;
	default:
		fmat = 0x00;
		break;
	}

	switch (iface->rate)
	{
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
		sample_fre = 44100;
	}
	regval = ac101_read_reg(I2S1LCK_CTRL);
	regval &= 0xffc3;
	regval |= (iface->mode << 15);
	regval |= (bits << 4);
	regval |= (fmat << 2);
	res |= ac101_write_reg(I2S1LCK_CTRL, regval);
	res |= ac101_write_reg(I2S_SR_CTRL, sample_fre);
	return res;
}

error_t AC101_i2s_config_clock(ac_i2s_clock_t *cfg)
{
	error_t res = 0;
	uint16_t regval = 0;
	regval = ac101_read_reg(I2S1LCK_CTRL);
	regval &= 0xe03f;
	regval |= (cfg->bclk_div << 9);
	regval |= (cfg->lclk_div << 6);
	res = ac101_write_reg(I2S1LCK_CTRL, regval);
	return res;
}

error_t ac101_set_voice_mute(bool enable)
{
	error_t res = 0;

	if (enable)
	{
		res = ac101_set_earph_volume(0);
		res |= ac101_set_spk_volume(0);
	}
	return res;
}

error_t ac101_set_voice_volume(int volume)
{
	error_t res;
	res = ac101_set_earph_volume(volume);
	res |= ac101_set_spk_volume(volume);
	return res;
}

error_t ac101_get_voice_volume(int *volume)
{
	*volume = ac101_get_earph_volume();
	return 0;
}


