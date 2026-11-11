
#pragma once
#include "ConfigAudioDriver.h"
#if AUDIO_DRIVER_LOGGIN_ACTVIE && !defined(AUDIO_DRIVER_LOGGING_IDF)

#include "LoggerSTD.h"
#include "etc.h"

#ifdef ARDUINO
#  include "Print.h"
#endif
// define supported log levels
enum class AudioDriverLogLevel {Debug=0, Info=1, Warning=2, Error=3};


/***
 * C++ API to configure logging
 */
class AudioDriverLoggerClass {
public:
    void setLogLevel(AudioDriverLogLevel level){
        LOGLEVEL_AUDIODRIVER = (int) level;
    }
#ifdef ARDUINO
    void setOutput(Print& out) {
        setAudioDriverLogOutput(&out);
    }
    bool begin(Print& out, AudioDriverLogLevel level) {
        setLogLevel(level);
        setOutput(out);
        return true;
    }
#endif
    bool begin(AudioDriverLogLevel level) {
        setLogLevel(level);
        return true;
    }
    void end(){
        setAudioDriverLogOutput(nullptr);
    }
};

// global object for std logging api
static AudioDriverLoggerClass AudioDriverLogger;

#endif