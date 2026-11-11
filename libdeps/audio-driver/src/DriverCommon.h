#pragma once
#include <stdint.h>

#include "ConfigAudioDriver.h"
#include "Platforms/Logger.h"

/*!
 * @file
 * @defgroup audio_driver Audio Driver
 * @defgroup enumerations Public enumeration types
 */


/// Fixed Definitions
#define RESULT_OK 0    /*!< error_t value indicating success (no error) */
#define RESULT_FAIL -1 /*!< Generic error_t code indicating failure */
#define ERROR_INVALID_ARG 1

#define I2C_END true   // wether to send a stop bit at the end of the transmission

#ifdef __cplusplus
#include "Platforms/AudioDriverLogger.h"

namespace audio_driver {
#endif

typedef int error_t;
typedef void *i2c_bus_handle_t;
typedef void *spi_bus_handle_t;

/**
 * @enum input_device_t
 * @brief Select adc for input mic signal. If the chip only has one ADC this might provide the 
 * functionality on the channel level
 * @ingroup enumerations
 * @ingroup audio_driver
 */
typedef enum {
  ADC_INPUT_NONE = 0x00, /*!< no input */
  ADC_INPUT_LINE1,       /*!< mic input from adc 1 */
  ADC_INPUT_LINE2,       /*!< mic input from adc 2 */
  ADC_INPUT_LINE3,       /*!< mic input from adc 3 */
  ADC_INPUT_ALL,         /*!< mic input from all adc */
  ADC_INPUT_DIFFERENCE,  /*!< mic input to adc difference channel */
} input_device_t;

/**
 * @enum output_device_t
 * @brief Select individual dac for dac output. If the device has only one DAC this might
 * provide the functionality channel level
 * @ingroup enumerations
 * @ingroup audio_driver
 */
typedef enum {
  DAC_OUTPUT_NONE = 0x00, /*!< no output */
  DAC_OUTPUT_LINE1,       /*!< dac output signal to dac 1 */
  DAC_OUTPUT_LINE2,       /*!< dac output signal to dac 2 */
  DAC_OUTPUT_ALL,         /*!< dac output signal to both dacs */
} output_device_t;

/**
 * @enum i2s_master_slave_t
 * @brief Select I2S interface operating mode i.e. master or slave for audio
 * codec chip
 * @ingroup enumerations
 */
typedef enum {
  MODE_SLAVE = 0x00,  /*!< set slave mode */
  MODE_MASTER = 0x01, /*!< set master mode */
} i2s_master_slave_t;

/**
 * @enum samplerate_t
 * @brief Select I2S interface samples per second
 * @ingroup enumerations
 */
typedef enum {
  RATE_8K = 0, /*!< set to  8k samples per second */
  RATE_11K,    /*!< set to 11.025k samples per second */
  RATE_16K,    /*!< set to 16k samples in per second */
  RATE_22K,    /*!< set to 22.050k samples per second */
  RATE_24K,    /*!< set to 24k samples in per second */
  RATE_32K,    /*!< set to 32k samples in per second */
  RATE_44K,    /*!< set to 44.1k samples per second */
  RATE_48K,    /*!< set to 48k samples per second */
  RATE_64K,    /*!< set to 64k samples per second */
  RATE_88K,    /*!< set to 88.2k samples per second */
  RATE_96K,    /*!< set to 96k samples per second */
  RATE_128K,   /*!< set to 128K samples per second */
  RATE_176K,   /*!< set to 176.4K samples per second */
  RATE_192K,   /*!< set to 192k samples per second */
} samplerate_t;

/**
 * @enum sample_bits_t
 * @brief Select I2S interface number of bits per sample
 * @ingroup enumerations
 */
typedef enum {
  BIT_LENGTH_MIN = -1,
  BIT_LENGTH_16BITS = 0x03,
  BIT_LENGTH_18BITS = 0x02,
  BIT_LENGTH_20BITS = 0x01,
  BIT_LENGTH_24BITS = 0x00,
  BIT_LENGTH_32BITS = 0x04,
  BIT_LENGTH_MAX,
} sample_bits_t;

/**
 * @enum i2s_format_t
 * @brief Select I2S interface format for audio codec chip
 * @ingroup enumerations
 */
typedef enum {
  I2S_NORMAL = 0, /*!< set normal I2S format */
  I2S_LEFT = 1,   /*!< set all left format */
  I2S_RIGHT = 2,  /*!< set all right format */
  I2S_DSP = 3,    /*!< set dsp/pcm format */
} i2s_format_t;

/**
 * @enum signal_t
 * @brief Usually the value is digital for i2s
 * @ingroup enumerations
 */

typedef enum {
  SIGNAL_DIGITAL,
  SIGNAL_AMALOG,
  SIGNAL_PDM,
  SIGNAL_TDM,
} signal_t;


/**
 * @enum es_mic_gain_t
 * @brief Microphone Gain
 * @ingroup enumerations
 */
typedef enum {
  MIC_GAIN_MIN = -1,
  MIC_GAIN_0DB = 0,
  MIC_GAIN_3DB = 3,
  MIC_GAIN_6DB = 6,
  MIC_GAIN_9DB = 9,
  MIC_GAIN_12DB = 12,
  MIC_GAIN_15DB = 15,
  MIC_GAIN_18DB = 18,
  MIC_GAIN_21DB = 21,
  MIC_GAIN_24DB = 24,
  MIC_GAIN_MAX,
} es_mic_gain_t;

/**
 * @enum codec_mode_t
 * @brief Select media hal codec mode
 * @ingroup enumerations
 */
typedef enum {
  CODEC_MODE_MIN = -1,
  CODEC_MODE_NONE = 0x00,
  CODEC_MODE_ENCODE = 0x01,  /*!< select adc */
  CODEC_MODE_DECODE = 0x02,  /*!< select dac */
  CODEC_MODE_BOTH = 0x03,    /*!< select both adc and dac */
  CODEC_MODE_LINE_IN = 0x04, /*!< set adc channel */
  CODEC_MODE_MAX
} codec_mode_t;

/**
 * @enum channels_t
 * @brief Select the number of channels
 * @ingroup enumerations
 */
typedef enum {
  CHANNELS2 = 2,
  CHANNELS4 = 4,
  CHANNELS8 = 8,
  CHANNELS16 = 16,
} channels_t;


/**
 * @brief I2s interface configuration for audio codec chip
 * @ingroup audio_driver
 */
typedef struct {
  /*!< Audio codec chip mode: if the microcontroller is master the codec must be
   * slave! */
  i2s_master_slave_t mode;
  /*!< I2S interface format */
  i2s_format_t fmt;
  /*!< I2S sample rate in samples per second */
  samplerate_t rate;
  /*!< i2s number of bits per sample */
  sample_bits_t bits;
  /*!< i2s number of channels */
  channels_t channels;
  /*!< signal tpye */
  signal_t signal_type;

} I2SDefinition;

/**
 * @brief Configure media hal for initialization of audio codec chip
 */
typedef struct {
  input_device_t input_device;   /*!< set adc channel */
  output_device_t output_device; /*!< set dac channel */
  I2SDefinition i2s;             /*!< set I2S interface configuration */
} codec_config_t;

#ifdef __cplusplus
}
// automatically use namespace
using namespace audio_driver;
#endif
