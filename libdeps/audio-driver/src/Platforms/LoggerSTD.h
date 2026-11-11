// logging methods that can be called from C and C++
#pragma once
#include "ConfigAudioDriver.h"

// maximum size of log string
#define AD_LOGLENGTH 215

// Logging of method
#define LOG_METHOD __PRETTY_FUNCTION__
// Log trace
#define AD_TRACED() AD_LOGD(LOG_METHOD)

#ifdef __cplusplus
extern "C" {
#endif

// define default log level
extern int LOGLEVEL_AUDIODRIVER;

void AD_LOGD(const char* fmr,...);
void AD_LOGI(const char* fmr,...);
void AD_LOGW(const char* fmr,...);
void AD_LOGE(const char* fmr,...);

void audioDriverLogStr(const char* msg);
void setAudioDriverLogOutput(void *out);


#ifdef __cplusplus
}
#endif


