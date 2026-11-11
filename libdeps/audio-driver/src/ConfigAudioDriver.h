#pragma once

// Default volume at startup
#ifndef DRIVER_DEFAULT_VOLUME
#  define DRIVER_DEFAULT_VOLUME 70
#endif

// AudioBoard getVolume() can report the volume reported by the driver or the volume that was last
// set. When using the driver volume, there might be small rounding errors compared to
// the last defined value.
#ifndef DRIVER_REPORT_DRIVER_VOLUME
#  define DRIVER_REPORT_DRIVER_VOLUME false
#endif


// To increase the max volume e.g. for ai_thinker (ES8388) 2957 or A202 -> set
// to 0, 1 or 2. 0: AUX volume is LINE level; 1: you can control the AUX volume with
// setVolume()
#ifndef AI_THINKER_ES8388_VOLUME_HACK
#  define AI_THINKER_ES8388_VOLUME_HACK 1
#endif

// Define the default gain for the microphone amp (see values from
// es_mic_gain_t) Alternativly you can call es8388_set_mic_gain(es_mic_gain_t
// gain) if you prefer to use value from an comprehensive enum
#ifndef ES8388_DEFAULT_INPUT_GAIN
#  define ES8388_DEFAULT_INPUT_GAIN 25
#endif

// For AI Thinker Audiokit we mute the power amplifier if the line is defined;
// set to -1 to deactivate this functionality
#ifndef ES8388_PA_LINE
#  define ES8388_PA_LINE 1
#endif

// If set to true, call Wire.close() on end: this might cause that the
// functionality can not be restarted
#ifndef FORCE_WIRE_CLOSE
#  define FORCE_WIRE_CLOSE false
#endif

/// Activate / Deactivate Logging: set to true or false
#ifndef AUDIO_DRIVER_LOGGIN_ACTVIE 
#  define AUDIO_DRIVER_LOGGIN_ACTVIE true
#endif

/// Define the default log level for the standard logger (2=Warning)
#ifndef AUDIO_DRIVER_LOG_LEVEL
#  define AUDIO_DRIVER_LOG_LEVEL 2
#endif 

/// Use IDF Logger: by default we use it if ESP32 outside of ARDUINO 
#if !defined(AUDIO_DRIVER_LOGGING_IDF) && defined(ESP32_CMAKE)
#  define AUDIO_DRIVER_LOGGING_IDF
#endif

/// Force to use the IDF I2C Implementation instead of the Arduino Wire
#if AUDIO_DRIVER_FORCE_IDF
#  define AUDIO_DRIVER_FORCE_IDF false
#endif