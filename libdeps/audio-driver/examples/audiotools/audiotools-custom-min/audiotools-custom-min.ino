/**
 * @brief We define a custom board w/o any pins and output_device a sine tone
 * with the help of the AudioTools I2SCodecStream
 * @author phil schatzmann
 */

#include "AudioTools.h" // install https://github.com/pschatzmann/arduino-audio-tools
#include "AudioLibs/I2SCodecStream.h"

AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> sound(sineWave);
AudioBoard board(AudioDriverES8388);
I2SCodecStream out(board);
StreamCopy copier(out, sound);

void setup() {
  // Setup logging
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  AudioDriverLogger.begin(Serial,AudioDriverLogLevel::Info); 

  // initialize i2c because board has no i2c definition
  Wire.begin();

  // start I2S & codec
  Serial.println("starting I2S...");
  auto config = out.defaultConfig();
  config.copyFrom(info);
  // define your i2s pins if you dont want to use the default pins
  config.pin_bck = 14;
  config.pin_ws = 15;
  config.pin_data = 22;
  out.begin(config);

  // Setup sine wave
  sineWave.begin(info, N_B4);
}

// Arduino loop - copy sound to out
void loop() { copier.copy(); }