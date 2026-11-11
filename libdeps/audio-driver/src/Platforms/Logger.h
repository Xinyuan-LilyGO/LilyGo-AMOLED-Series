#pragma once
#include "ConfigAudioDriver.h"

#if AUDIO_DRIVER_LOGGIN_ACTVIE
#  if defined(AUDIO_DRIVER_LOGGING_IDF)
#    include "LoggerIDF.h"
#  else
#    include "LoggerSTD.h"
#  endif
#else

#  define AD_LOGD(...) 
#  define AD_LOGI(...)
#  define AD_LOGW(...)
#  define AD_LOGE(...)

#  define AD_TRACED()

#endif