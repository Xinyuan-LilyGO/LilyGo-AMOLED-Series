#pragma once
#include "Driver.h"
#include "DriverPins.h"

namespace audio_driver {

/**
 * @brief Defitintion for audio board pins and an audio driver
 * @ingroup audio_driver
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class AudioBoard {
public:

  AudioBoard(AudioDriver *driver, DriverPins* pins) {
    setPins(*pins);
    setDriver(*driver);
  }
  
  AudioBoard(AudioDriver &driver, DriverPins& pins) {
    setPins(pins);
    setDriver(driver);
  }


  bool begin(){
    AD_LOGD("AudioBoard::begin");
    if (p_pins==nullptr){
      AD_LOGE("pins are null");
      return false;
    }
    if (!p_driver->begin(codec_cfg, *p_pins)){
      AD_LOGE("AudioBoard::driver::begin failed");
      return false;
    }
    setVolume(DRIVER_DEFAULT_VOLUME);
    is_active = true;
    return true;
  }

  /// Starts the processing
  bool begin(CodecConfig cfg) {
    this->codec_cfg = cfg;
    return begin();
  }

  /// Updates the CodecConfig values -> reconfigures the codec only
  bool setConfig(CodecConfig cfg) {
    this->codec_cfg = cfg;
    return p_driver->setConfig(cfg);
  }

  bool end(void) {
    p_pins->end();
    is_active = false;
    return p_driver->end();
  }
  bool setMute(bool enable) { return p_driver->setMute(enable); }
  bool setMute(bool enable, int line) { 
    if (line == power_amp_line) setPAPower(!enable);
    return p_driver->setMute(enable, line); 
  }
  bool setVolume(int volume) {
    AD_LOGD("setVolume: %d", volume);
    // when we get the volume we make sure that we report the same value
    // w/o rounding issues 
    this->volume = volume; 
    return (is_active) ? p_driver->setVolume(volume) : false; 
  }
  int getVolume() { 
#if DRIVER_REPORT_DRIVER_VOLUME
    return driver->getVolume(); }
#else
    return volume >= 0 ? volume : p_driver->getVolume(); }
#endif

  void setPins(DriverPins&pins){
    this->p_pins = &pins;
  }
  DriverPins& getPins() { return *p_pins; }
  DriverPins& pins() { return *p_pins; }

  void setDriver(AudioDriver& driver){
    this->p_driver = &driver;
  }

  AudioDriver* getDriver(){
    return p_driver;
  }
  AudioDriver& driver(){
    return *p_driver;
  }

  bool setPAPower(bool enable) { return is_active ? p_driver->setPAPower(enable) : false; }
  
  /// set volume for adc: this is only supported on some defined codecs
  bool setInputVolume(int volume) {return p_driver->setInputVolume(volume);}
 
  // platform specific logic to determine if key is pressed
  bool isKeyPressed(uint8_t key) { return p_pins->isKeyPressed(key); }

  operator bool() { return is_active && p_driver != nullptr && p_pins != nullptr;}

protected:
  AudioDriver* p_driver = nullptr;
  DriverPins* p_pins = nullptr;
  CodecConfig codec_cfg;
  int power_amp_line = ES8388_PA_LINE;
  int volume = -1;
  bool is_active = false;
};


// -- Boards
/// @ingroup audio_driver
static AudioBoard AudioKitEs8388V1{AudioDriverES8388, PinsAudioKitEs8388v1};
/// @ingroup audio_driver
static AudioBoard AudioKitEs8388V2{AudioDriverES8388, PinsAudioKitEs8388v2};
/// @ingroup audio_driver
static AudioBoard AudioKitAC101{AudioDriverAC101, PinsAudioKitAC101};
/// @ingroup audio_driver
static AudioBoard LyratV43{AudioDriverES8388H3, PinsLyrat43};
/// @ingroup audio_driver
static AudioBoard LyratV42{AudioDriverES8388H3, PinsLyrat42};
/// @ingroup audio_driver
static AudioBoard LyratMini{AudioDriverLyratMini, PinsLyratMini};
/// @ingroup audio_driver
static AudioBoard NoBoard{NoDriver, NoPins};
/// @ingroup audio_driver
static AudioBoard GenericWM8960{AudioDriverWM8960, NoPins};
/// @ingroup audio_driver
static AudioBoard GenericCS43l22{AudioDriverCS43l22, NoPins};


}