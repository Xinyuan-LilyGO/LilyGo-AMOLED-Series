/**
 * @brief We use a predefied board (AudioKitEs8388V1) and output_device a sine
 * with the help of the AudioTools I2SCodecStream
 * @author phil schatzmann
 */

#include "AudioTools.h" // install https://github.com/pschatzmann/arduino-audio-tools
#include "AudioLibs/I2SCodecStream.h"

AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);
I2SCodecStream out(AudioKitEs8388V1);
StreamCopy copier(out, sound);

void setup() {
  // Setup logging
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  AudioDriverLogger.begin(Serial,AudioDriverLogLevel::Info); 

  // start I2S & codec with i2c and i2s configured above
  Serial.println("starting I2S...");
  auto config = out.defaultConfig();
  config.copyFrom(info);
  out.begin(config);

  // Setup sine wave
  sineWave.begin(info, N_B4);
}

// Arduino loop - copy sound to out
void loop() { copier.copy(); }