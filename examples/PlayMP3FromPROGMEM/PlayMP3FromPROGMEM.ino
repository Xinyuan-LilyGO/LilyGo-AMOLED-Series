/*
 * Standalone MP3 playback test for the T-Display-S3 AMOLED Plus Shield.
 *
 * Only the T-Display-S3 AMOLED Plus with the matching Shield is supported.
 * Other AMOLED Series boards do not have the required ES8311 audio path.
 */

#include <LilyGo_AMOLED.h>

#include <AudioBoard.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <expander/IoExpanderXL9555.hpp>

#include "VanLifeMp3.h"

namespace {
constexpr int kI2sWs = 40;
constexpr int kI2sSck = 44;
constexpr int kI2sMclk = 42;
constexpr int kI2sSdout = 43;
constexpr int kI2sSdin = 41;
constexpr uint8_t kPaEnablePin = 2;

LilyGo_Class amoled;
IoExpanderXL9555 expander;
DriverPins audioPins;
AudioBoard codec(AudioDriverES8311, audioPins);
AudioFileSourcePROGMEM source(kVanLifeMp3, kVanLifeMp3Size);
AudioGeneratorMP3 decoder;
AudioOutputI2S *output = nullptr;
bool playbackFinished = false;

void fail(const char *message)
{
    Serial.println(message);
    while (true) delay(1000);
}

void setAmplifier(bool enabled)
{
    expander.digitalWrite(kPaEnablePin, enabled ? HIGH : LOW);
}
}

void setup()
{
    Serial.begin(115200);
    if (!amoled.begin()) fail("AMOLED board detection failed");

    const BoardsConfigure_t *board = amoled.getBoardsConfigure();
    const int sda = board->touch ? board->touch->sda :
                    (board->pmu ? board->pmu->sda :
                     (board->sensor ? board->sensor->sda : -1));
    const int scl = board->touch ? board->touch->scl :
                    (board->pmu ? board->pmu->scl :
                     (board->sensor ? board->sensor->scl : -1));
    if (sda < 0 || scl < 0) fail("Board has no shared I2C bus");
    Wire.begin(sda, scl);
    Wire.setClock(400000);

    if (!expander.begin(Wire, XL9555_UNKNOWN_ADDRESS)) {
        fail("AMOLED Plus Shield XL9555 not found");
    }
    expander.pinMode(kPaEnablePin, OUTPUT);
    setAmplifier(false);

    audioPins.addI2C(PinFunction::CODEC, Wire);
    audioPins.addI2S(PinFunction::CODEC, kI2sMclk, kI2sSck, kI2sWs, kI2sSdout, kI2sSdin);
    CodecConfig config;
    config.input_device = ADC_INPUT_LINE1;
    config.output_device = DAC_OUTPUT_ALL;
    config.i2s.bits = BIT_LENGTH_16BITS;
    config.i2s.rate = RATE_44K;
    if (!codec.begin(config)) fail("ES8311 initialization failed");
    codec.setVolume(80);
    codec.setMute(false);

    output = new AudioOutputI2S(0, AudioOutputI2S::EXTERNAL_I2S);
    if (!output) fail("I2S output allocation failed");
    output->SetPinout(kI2sSck, kI2sWs, kI2sSdout, kI2sMclk);
    output->SetGain(0.8f);

    setAmplifier(true);
    delay(5);
    if (!decoder.begin(&source, output)) {
        setAmplifier(false);
        fail("MP3 decoder failed to start");
    }
    Serial.printf("Playing embedded mono MP3 (%u bytes)\n", (unsigned)kVanLifeMp3Size);
}

void loop()
{
    if (decoder.isRunning()) {
        if (!decoder.loop()) decoder.stop();
        return;
    }
    if (!playbackFinished) {
        playbackFinished = true;
        setAmplifier(false);
        Serial.println("MP3 playback complete; amplifier disabled");
    }
    delay(10);
}
