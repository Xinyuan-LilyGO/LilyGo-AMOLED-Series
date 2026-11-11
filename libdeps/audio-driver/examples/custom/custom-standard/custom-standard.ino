/**
 * @brief We just set up the codec for a predefined board (AudioKitEs8388V1)
 * @author phil schatzmann
 */

#include "AudioBoard.h"

void setup() {
  // Setup logging
  Serial.begin(115200);
  AudioDriverLogger.begin(Serial,AudioDriverLogLevel::Info); 

  // configure codec
  CodecConfig cfg;
  cfg.input_device = ADC_INPUT_LINE1;
  cfg.output_device = DAC_OUTPUT_ALL;
  cfg.i2s.bits = BIT_LENGTH_16BITS;
  cfg.i2s.rate = RATE_44K;
  // cfg.i2s.fmt = I2S_NORMAL;
  // cfg.i2s.mode = MODE_SLAVE;
  AudioKitEs8388V1.begin(cfg);
}

void loop() {}
