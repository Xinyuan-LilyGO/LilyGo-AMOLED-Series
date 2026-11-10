#include "FactoryShield.h"

#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <driver/rmt_encoder.h>
#include <driver/rmt_rx.h>
#include <driver/rmt_tx.h>
#include <limits.h>
#include <math.h>
#include <new>
#include <stdarg.h>
#include <string.h>
#include <AudioBoard.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorRTTTL.h>
#include <RadioLib.h>
#include <AudioOutputI2S.h>
#include <expander/IoExpanderXL9555.hpp>
#include <SensorQMI8658.hpp>
#include "audio/VanLifeMp3.h"
#include "sensor/magnetometer/qmc/SensorQMC6309.hpp"

namespace {
constexpr uint8_t kShieldAddress = 0x20;
constexpr int kLoraSclk = 14;
constexpr int kLoraMiso = 13;
constexpr int kLoraMosi = 12;
constexpr int kLoraNss = 45;
constexpr int kLoraDio1 = 10;
constexpr int kLoraReset = 46;
constexpr int kLoraBusy = 16;
constexpr int kIrPin = 1;
constexpr int kIrSelectPin = 39;
constexpr uint32_t kIrTestNecCode = 0x20DF10EFUL;
constexpr uint32_t kIrTxIntervalMs = 1000;
constexpr int kI2sWs = 40;
constexpr int kI2sSck = 44;
constexpr int kI2sMclk = 42;
constexpr int kI2sSdout = 43;
constexpr int kI2sSdin = 41;
constexpr uint8_t kEs8311Address = 0x18;
constexpr uint32_t kMagCalibrationDurationMs = 15000;
constexpr int32_t kMagCalibrationMinSpan = 200;

IoExpanderXL9555 shieldExpander;
bool shieldExpanderReady = false;
bool audioAmplifierEnabled = false;
Module loraModule(kLoraNss, kLoraDio1, kLoraReset, kLoraBusy);
SX1262 lora(&loraModule);

// ESP8266Audio follows each stream's sample rate. Keep the slave ES8311 clock
// dividers synchronized with the ESP32 I2S rate whenever that value changes.
class CodecSynchronizedAudioOutput final : public AudioOutputI2S {
public:
    CodecSynchronizedAudioOutput() : AudioOutputI2S()
    {
#ifdef ESP32
        _useAPLL = false;
#endif
        // FactoryV2 can spend longer than a small audio DMA window rendering a
        // full LVGL frame. Keep enough audio queued to cover those bursts.
        SetBuffers(12, 2048);
    }

    bool SetRate(int hz) override;
};

AudioOutputI2S *audioOutput = nullptr;
AudioGenerator *activeAudioGenerator = nullptr;
AudioGeneratorRTTTL *toneGenerator = nullptr;
AudioGeneratorMP3 *mp3Generator = nullptr;
AudioFileSourcePROGMEM *programAudioSource = nullptr;
AudioFileSourceSD *sdAudioSource = nullptr;
DriverPins audioPins;
AudioBoard audioBoard(AudioDriverES8311, audioPins);
CodecConfig audioCodecConfig;
int audioCodecSampleRate = 44100;
bool audioRateSyncFailed = false;
SensorQMI8658 shieldImu;
SensorQMC6309 shieldMagnetometer;
volatile bool loraIrqFlag = false;
constexpr size_t kIrRxSymbolCapacity = 64;
rmt_channel_handle_t irRxChannel = nullptr;
rmt_symbol_word_t irRxSymbols[kIrRxSymbolCapacity] = {};
volatile size_t irRxSymbolCount = 0;
volatile bool irRxFrameReady = false;
constexpr size_t kIrTxSymbolCapacity = 35;
rmt_channel_handle_t irTxChannel = nullptr;
rmt_encoder_handle_t irTxEncoder = nullptr;
rmt_symbol_word_t irTxSymbols[kIrTxSymbolCapacity] = {};
volatile bool irTxFrameDone = false;
constexpr i2s_port_t kMicPort = I2S_NUM_1;
const char kToneBeep[] PROGMEM = "Beep:d=8,o=5,b=160:g5,g5,2c6";
i2s_chan_handle_t micRxChannel = nullptr;
i2s_chan_handle_t micTxChannel = nullptr;

void loraIrq()
{
    loraIrqFlag = true;
}

void setStatusText(char *buffer, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);
}

float normalizeHeading(float heading)
{
    heading = fmodf(heading, 360.0f);
    return heading < 0.0f ? heading + 360.0f : heading;
}

void setAudioAmplifierPower(bool enabled)
{
    if (!shieldExpanderReady || audioAmplifierEnabled == enabled) return;
    shieldExpander.digitalWrite(2, enabled ? HIGH : LOW);
    audioAmplifierEnabled = enabled;
}

int supportedCodecSampleRate(int requested)
{
    static constexpr int kRates[] = {8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000};
    int selected = kRates[0];
    int difference = abs(requested - selected);
    for (const int rate : kRates) {
        const int candidateDifference = abs(requested - rate);
        if (candidateDifference < difference) {
            selected = rate;
            difference = candidateDifference;
        }
    }
    return selected;
}

bool CodecSynchronizedAudioOutput::SetRate(int requestedRate)
{
    const int effectiveRate = supportedCodecSampleRate(requestedRate);
    const bool i2sReady = AudioOutputI2S::SetRate(effectiveRate);
    if (!i2sReady) {
        audioRateSyncFailed = true;
        return false;
    }
    if (audioCodecSampleRate == effectiveRate) {
        audioRateSyncFailed = false;
        return true;
    }

    CodecConfig nextConfig = audioCodecConfig;
    nextConfig.setRateNumeric(effectiveRate);
    audioBoard.setMute(true);
    const bool codecReady = audioBoard.setConfig(nextConfig);
    if (codecReady) {
        audioCodecConfig = nextConfig;
        audioCodecSampleRate = effectiveRate;
        audioBoard.setVolume(audioBoard.getVolume());
        audioBoard.setInputVolume(20);
    }
    audioBoard.setMute(false);
    audioRateSyncFailed = !codecReady;
    return codecReady;
}

// Put the shared IR line in a high-impedance state and select the receiver
// path so a completed TX cannot look like a continuously illuminated LED.
void releaseIrTransmitter()
{
    // Match the shield's standalone test: make the IR line inactive before
    // switching the analog path back to the receiver.
    pinMode(kIrPin, OUTPUT);
    digitalWrite(kIrPin, LOW);
    pinMode(kIrPin, INPUT);
    digitalWrite(kIrSelectPin, HIGH);
}

bool IRAM_ATTR irTxDone(rmt_channel_handle_t, const rmt_tx_done_event_data_t *, void *)
{
    irTxFrameDone = true;
    return false;
}

void stopIrRmtTransmitter()
{
    if (irTxChannel) {
        rmt_disable(irTxChannel);
        rmt_del_channel(irTxChannel);
        irTxChannel = nullptr;
    }
    if (irTxEncoder) {
        rmt_del_encoder(irTxEncoder);
        irTxEncoder = nullptr;
    }
    irTxFrameDone = false;
}

// Submit the complete NEC frame and return immediately. The symbol buffer and
// RMT handles remain valid until pollIr() observes the completion callback.
bool startIrNecRmt(uint32_t data)
{
    stopIrRmtTransmitter();
    memset(irTxSymbols, 0, sizeof(irTxSymbols));
    size_t count = 0;
    irTxSymbols[count++] = {9000, 1, 4500, 0};
    for (int bit = 31; bit >= 0; --bit) {
        const uint16_t space = (data & (1UL << bit)) ? 1690U : 560U;
        irTxSymbols[count++] = {560, 1, space, 0};
    }
    irTxSymbols[count++] = {560, 1, 20000, 0};
    irTxSymbols[count++] = {20000, 0, 1, 0};

    rmt_tx_channel_config_t channelConfig = {};
    channelConfig.gpio_num = (gpio_num_t)kIrPin;
    channelConfig.clk_src = RMT_CLK_SRC_DEFAULT;
    channelConfig.resolution_hz = 1000000;
    channelConfig.mem_block_symbols = 64;
    channelConfig.trans_queue_depth = 1;
    channelConfig.flags.init_level = 0;
    esp_err_t state = rmt_new_tx_channel(&channelConfig, &irTxChannel);

    rmt_carrier_config_t carrier = {};
    carrier.frequency_hz = 38000;
    carrier.duty_cycle = 0.33f;
    if (state == ESP_OK) state = rmt_apply_carrier(irTxChannel, &carrier);
    if (state == ESP_OK) {
        rmt_copy_encoder_config_t encoderConfig = {};
        state = rmt_new_copy_encoder(&encoderConfig, &irTxEncoder);
    }
    if (state == ESP_OK) {
        rmt_tx_event_callbacks_t callbacks = {};
        callbacks.on_trans_done = irTxDone;
        state = rmt_tx_register_event_callbacks(irTxChannel, &callbacks, nullptr);
    }
    if (state == ESP_OK) state = rmt_enable(irTxChannel);
    if (state == ESP_OK) {
        rmt_transmit_config_t transmitConfig = {};
        state = rmt_transmit(irTxChannel, irTxEncoder, irTxSymbols,
                             count * sizeof(irTxSymbols[0]), &transmitConfig);
    }
    if (state != ESP_OK) {
        stopIrRmtTransmitter();
        return false;
    }
    return true;
}

bool IRAM_ATTR irRxDone(rmt_channel_handle_t, const rmt_rx_done_event_data_t *event, void *)
{
    irRxSymbolCount = event->num_symbols <= kIrRxSymbolCapacity ?
                      event->num_symbols : kIrRxSymbolCapacity;
    irRxFrameReady = true;
    return false;
}

void stopIrRmtReceiver()
{
    if (irRxChannel) {
        rmt_disable(irRxChannel);
        rmt_del_channel(irRxChannel);
        irRxChannel = nullptr;
    }
    irRxSymbolCount = 0;
    irRxFrameReady = false;
    pinMode(kIrPin, INPUT);
}

bool armIrRmtReceiver()
{
    if (!irRxChannel || irRxFrameReady) return false;
    rmt_receive_config_t receiveConfig = {};
    receiveConfig.signal_range_min_ns = 1250;
    receiveConfig.signal_range_max_ns = 15000000;
    const esp_err_t state = rmt_receive(irRxChannel, irRxSymbols, sizeof(irRxSymbols),
                                        &receiveConfig);
    if (state != ESP_OK) {
        return false;
    }
    return true;
}

bool startIrRmtReceiver()
{
    stopIrRmtReceiver();
    rmt_rx_channel_config_t channelConfig = {};
    channelConfig.gpio_num = (gpio_num_t)kIrPin;
    channelConfig.clk_src = RMT_CLK_SRC_DEFAULT;
    channelConfig.resolution_hz = 1000000;
    channelConfig.mem_block_symbols = 64;
    esp_err_t state = rmt_new_rx_channel(&channelConfig, &irRxChannel);
    if (state == ESP_OK) {
        rmt_rx_event_callbacks_t callbacks = {};
        callbacks.on_recv_done = irRxDone;
        state = rmt_rx_register_event_callbacks(irRxChannel, &callbacks, nullptr);
    }
    if (state == ESP_OK) state = rmt_enable(irRxChannel);
    if (state == ESP_OK && armIrRmtReceiver()) return true;

    stopIrRmtReceiver();
    return false;
}

bool durationMatches(uint16_t actual, uint16_t expected)
{
    constexpr uint16_t kTolerancePercent = 35;
    const uint32_t minimum = (uint32_t)expected * (100 - kTolerancePercent) / 100;
    const uint32_t maximum = (uint32_t)expected * (100 + kTolerancePercent) / 100;
    return actual >= minimum && actual <= maximum;
}

bool decodeNecRmt(const rmt_symbol_word_t *symbols, size_t count, uint32_t &value)
{
    if (count < 34 || !durationMatches(symbols[0].duration0, 9000) ||
        !durationMatches(symbols[0].duration1, 4500)) {
        return false;
    }

    uint32_t decoded = 0;
    for (size_t i = 1; i <= 32; ++i) {
        if (!durationMatches(symbols[i].duration0, 560)) return false;
        decoded <<= 1;
        if (durationMatches(symbols[i].duration1, 1690)) {
            decoded |= 1;
        } else if (!durationMatches(symbols[i].duration1, 560)) {
            return false;
        }
    }
    if (!durationMatches(symbols[33].duration0, 560)) return false;

    const uint8_t command = decoded >> 8;
    const uint8_t commandInverse = decoded;
    // Both standard and extended NEC require an inverted command byte. The
    // address bytes may be complements or a 16-bit extended address.
    if ((uint8_t)(command ^ commandInverse) != 0xFF) return false;
    value = decoded;
    return true;
}

}

bool FactoryShield::probe(TwoWire &wire, uint8_t address)
{
    wire.beginTransmission(address);
    return wire.endTransmission() == 0;
}

bool FactoryShield::startLoraPacket()
{
    char payload[32];
    snprintf(payload, sizeof(payload), "%lu", (unsigned long)loraTxSequence_++);
    snprintf(loraTxMessage_, sizeof(loraTxMessage_), "%s", payload);
    if (lora.startTransmit(loraTxMessage_) != RADIOLIB_ERR_NONE) return false;
    loraTxActive_ = true;
    ++loraTxCount_;
    return true;
}

bool FactoryShield::begin(TwoWire &wire, int sda, int scl)
{
    present_ = false;
    loraReady_ = false;
    codecReady_ = false;
    audioReady_ = false;
    irReady_ = false;
    sensorReady_ = false;
    imuReady_ = false;
    magnetometerReady_ = false;
    micReady_ = false;
    micChannelsEnabled_ = false;
    micActive_ = false;
    audioActive_ = false;
    audioFault_ = false;
    audioRateSyncFailed = false;
    audioCodecSampleRate = 44100;
    micFault_ = false;
    audioVolume_ = FACTORY_DEFAULT_AUDIO_VOLUME;
    loraTxActive_ = false;
    loraRxActive_ = false;
    loraFault_ = false;
    loraTxCount_ = 0;
    loraTxSequence_ = 0;
    loraRxCount_ = 0;
    loraTxPeriodic_ = false;
    irReceiveActive_ = false;
    irTxPending_ = false;
    irTxActive_ = false;
    irTxRunning_ = false;
    irFault_ = false;
    irTxUntilMs_ = 0;
    irTxNextMs_ = 0;
    irTxCount_ = 0;
    irMatch_ = false;
    irIsNec_ = false;
    irBits_ = 0;
    irCode_ = 0;
    irRaw_[0] = '\0';
    micLastDataMs_ = 0;
    magHeading_ = 0;
    magUnalignedHeading_ = 0;
    magHeadingOffset_ = 0;
    magScaleX_ = magScaleY_ = magScaleZ_ = 1.0f;
    magOffsetX_ = magOffsetY_ = magOffsetZ_ = 0;
    magCalibrating_ = false;
    magCalibrated_ = false;
    magNorthAlignmentPending_ = false;
    magCalibrationProgress_ = 0;
    magCalibrationStartedMs_ = 0;
    status_[0] = '\0';
    loraTxMessage_[0] = '\0';
    loraRxMessage_[0] = '\0';
    if (sda < 0 || scl < 0) return false;

    wire.begin(sda, scl);
    if (!probe(wire, kShieldAddress)) {
        return false;
    }
    wire.setClock(400000);
    present_ = true;

    shieldExpanderReady = shieldExpander.begin(wire, XL9555_UNKNOWN_ADDRESS);
    audioAmplifierEnabled = false;
    if (shieldExpanderReady) {
        shieldExpander.pinMode(0, OUTPUT);  // LoRa power / enable
        shieldExpander.pinMode(2, OUTPUT);  // Audio power amplifier
        shieldExpander.digitalWrite(0, HIGH);
        shieldExpander.digitalWrite(2, LOW);
    }

    SPI.begin(kLoraSclk, kLoraMiso, kLoraMosi);
    const int loraState = lora.begin(loraFrequency_, loraBandwidth_, loraSpreadingFactor_,
                                     loraCodingRate_, 0x12, loraPower_, 8, 3.0, false);
    loraReady_ = loraState == RADIOLIB_ERR_NONE;
    if (loraReady_) {
        lora.setCurrentLimit(140);
        lora.setDio1Action(loraIrq);
    }

    const bool codecPresent = probe(wire, kEs8311Address);
    if (codecPresent) {
        audioPins.addI2C(PinFunction::CODEC, wire);
        audioPins.addI2S(PinFunction::CODEC, kI2sMclk, kI2sSck, kI2sWs, kI2sSdout, kI2sSdin);
        audioCodecConfig = CodecConfig();
        audioCodecConfig.input_device = ADC_INPUT_LINE1;
        audioCodecConfig.output_device = DAC_OUTPUT_ALL;
        audioCodecConfig.i2s.bits = BIT_LENGTH_16BITS;
        audioCodecConfig.i2s.rate = RATE_44K;
        codecReady_ = audioBoard.begin(audioCodecConfig);
        if (codecReady_) {
            audioBoard.setVolume(audioVolume_);
            audioBoard.setMute(false);
            audioBoard.setInputVolume(20);
        }
        audioOutput = new (std::nothrow) CodecSynchronizedAudioOutput();
        if (audioOutput && codecReady_) {
            audioOutput->SetPinout(kI2sSck, kI2sWs, kI2sSdout, kI2sMclk);
            audioOutput->SetGain(1.0f);
            audioReady_ = true;
        }
    }

    pinMode(kIrSelectPin, OUTPUT);
    digitalWrite(kIrSelectPin, HIGH);
    pinMode(kIrPin, INPUT);
    delay(5);
    irReady_ = true;

    i2s_chan_config_t micChannelConfig = I2S_CHANNEL_DEFAULT_CONFIG(kMicPort, I2S_ROLE_MASTER);
    micChannelConfig.dma_desc_num = 8;
    micChannelConfig.dma_frame_num = 128;
    const esp_err_t micInstall = i2s_new_channel(&micChannelConfig, &micTxChannel, &micRxChannel);
    esp_err_t micTxInit = ESP_ERR_INVALID_STATE;
    esp_err_t micRxInit = ESP_ERR_INVALID_STATE;
    if (micInstall == ESP_OK && micTxChannel && micRxChannel) {
        i2s_std_config_t micConfig = {
            .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                .mclk = (gpio_num_t)kI2sMclk,
                .bclk = (gpio_num_t)kI2sSck,
                .ws = (gpio_num_t)kI2sWs,
                .dout = (gpio_num_t)kI2sSdout,
                .din = (gpio_num_t)kI2sSdin,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv = false,
                },
            },
        };
        micConfig.slot_cfg.slot_mask = I2S_STD_SLOT_BOTH;
        micTxInit = i2s_channel_init_std_mode(micTxChannel, &micConfig);
        micRxInit = i2s_channel_init_std_mode(micRxChannel, &micConfig);
        micReady_ = micTxInit == ESP_OK && micRxInit == ESP_OK;
    }

    imuReady_ = shieldImu.begin(wire, QMI8658_H_SLAVE_ADDRESS);
    magnetometerReady_ = shieldMagnetometer.begin(wire, QMC6309_SLAVE_ADDRESS);
    if (imuReady_) {
        shieldImu.configAccelerometer(SensorQMI8658::ACC_RANGE_4G,
                                      SensorQMI8658::ACC_ODR_1000Hz,
                                      SensorQMI8658::LPF_MODE_0);
        shieldImu.configGyroscope(SensorQMI8658::GYR_RANGE_64DPS,
                                   SensorQMI8658::GYR_ODR_896_8Hz,
                                   SensorQMI8658::LPF_MODE_3);
        shieldImu.enableAccelerometer();
        shieldImu.enableGyroscope();
    }
    if (magnetometerReady_) {
        shieldMagnetometer.configMagnetometer(OperationMode::CONTINUOUS_MEASUREMENT,
                                               MagFullScaleRange::FS_8G, 200.0f,
                                               MagOverSampleRatio::OSR_1,
                                               MagDownSampleRatio::DSR_1);
    }
    sensorReady_ = imuReady_ || magnetometerReady_;
    setStatusText(status_, sizeof(status_), "Shield ready");
    return true;
}

void FactoryShield::update(FactorySnapshot &snapshot)
{
    if (!present_) return;

    if (irTxActive_ && !irTxChannel && (int32_t)(millis() - irTxUntilMs_) >= 0) {
        irTxActive_ = false;
    }

    if (micActive_ && micReady_) {
        static int16_t samples[256];
        static int16_t silence[1024] = {};
        size_t bytesWritten = 0;
        i2s_channel_write(micTxChannel, silence, sizeof(silence), &bytesWritten, 5);
        size_t bytesRead = 0;
        const esp_err_t readState = i2s_channel_read(micRxChannel, samples, sizeof(samples),
                                                      &bytesRead, 20);
        if (readState == ESP_OK && bytesRead) {
            int64_t sum = 0;
            const size_t count = bytesRead / sizeof(samples[0]);
            for (size_t i = 0; i < count; ++i) {
                const int32_t sample = samples[i];
                const int32_t magnitude = sample < 0 ? -sample : sample;
                sum += magnitude;
            }
            micLevel_ = count ? (int)(sum / count) : 0;
            for (size_t i = 0; i < 32; ++i) {
                const size_t source = (i * count) / 32;
                micWaveform_[i] = count ? samples[source] / 64 : 0;
            }
            micLastDataMs_ = millis();
            micFault_ = false;
            setStatusText(status_, sizeof(status_), "Mic %d", micLevel_);
        } else {
            micFault_ = true;
        }
        const uint32_t now = millis();
        if (micLastDataMs_ && now - micLastDataMs_ > 500) {
            micFault_ = true;
        }
    }

    if (loraIrqFlag) {
        loraIrqFlag = false;
        if (loraTxActive_) {
            lora.finishTransmit();
            loraTxActive_ = false;
            loraNextTxMs_ = millis() + loraTxIntervalMs_;
            setStatusText(status_, sizeof(status_), "LoRa TX complete");
        } else if (loraRxActive_) {
            String data;
            const int state = lora.readData(data);
            if (state == RADIOLIB_ERR_NONE) {
                snprintf(loraRxMessage_, sizeof(loraRxMessage_), "%s", data.c_str());
                loraRssi_ = (int16_t)lora.getRSSI();
                loraSnr_ = lora.getSNR();
                ++loraRxCount_;
                loraFault_ = false;
                setStatusText(status_, sizeof(status_), "LoRa RX: %s RSSI %d SNR %.1f", data.c_str(), (int)loraRssi_, loraSnr_);
            } else {
                loraFault_ = true;
                setStatusText(status_, sizeof(status_), "LoRa RX error %d", state);
            }
            lora.startReceive();
        }
    }

    if (loraTxPeriodic_ && !loraTxActive_ && !loraRxActive_ &&
        (int32_t)(millis() - loraNextTxMs_) >= 0) {
        loraTxActive_ = startLoraPacket();
        if (loraTxActive_) {
            loraFault_ = false;
            setStatusText(status_, sizeof(status_), "LoRa TX %s", loraTxMessage_);
        } else {
            loraFault_ = true;
        }
    }

    if (imuReady_ && shieldImu.getDataReady()) {
        float ax, ay, az, gx, gy, gz;
        shieldImu.getAccelerometer(ax, ay, az);
        shieldImu.getGyroscope(gx, gy, gz);
        imuAx_ = ax; imuAy_ = ay; imuAz_ = az;
        gyroGx_ = gx; gyroGy_ = gy; gyroGz_ = gz;
    }
    if (magnetometerReady_) {
        MagnetometerData magnetic;
        if (shieldMagnetometer.readData(magnetic)) {
            if (magCalibrating_) {
                magMinX_ = min(magMinX_, magnetic.raw.x);
                magMinY_ = min(magMinY_, magnetic.raw.y);
                magMinZ_ = min(magMinZ_, magnetic.raw.z);
                magMaxX_ = max(magMaxX_, magnetic.raw.x);
                magMaxY_ = max(magMaxY_, magnetic.raw.y);
                magMaxZ_ = max(magMaxZ_, magnetic.raw.z);
                const uint32_t elapsed = millis() - magCalibrationStartedMs_;
                magCalibrationProgress_ = min(100UL, elapsed * 100UL / kMagCalibrationDurationMs);
                if (elapsed >= kMagCalibrationDurationMs) {
                    const int32_t spanX = (int32_t)magMaxX_ - magMinX_;
                    const int32_t spanY = (int32_t)magMaxY_ - magMinY_;
                    const int32_t spanZ = (int32_t)magMaxZ_ - magMinZ_;
                    magCalibrating_ = false;
                    magCalibrationProgress_ = 100;
                    magCalibrated_ = spanX >= kMagCalibrationMinSpan &&
                                     spanY >= kMagCalibrationMinSpan;
                    if (magCalibrated_) {
                        magOffsetX_ = (int16_t)(((int32_t)magMinX_ + magMaxX_) / 2);
                        magOffsetY_ = (int16_t)(((int32_t)magMinY_ + magMaxY_) / 2);
                        magOffsetZ_ = (int16_t)(((int32_t)magMinZ_ + magMaxZ_) / 2);
                        const float halfX = spanX * 0.5f;
                        const float halfY = spanY * 0.5f;
                        const float averageHalf = (halfX + halfY) * 0.5f;
                        magScaleX_ = averageHalf / halfX;
                        magScaleY_ = averageHalf / halfY;
                        magScaleZ_ = spanZ >= kMagCalibrationMinSpan
                                         ? averageHalf / (spanZ * 0.5f)
                                         : 1.0f;
                        magNorthAlignmentPending_ = true;
                        setStatusText(status_, sizeof(status_),
                                      "Point device top North, then tap align");
                    } else {
                        magOffsetX_ = magOffsetY_ = magOffsetZ_ = 0;
                        magScaleX_ = magScaleY_ = magScaleZ_ = 1.0f;
                        magNorthAlignmentPending_ = false;
                        setStatusText(status_, sizeof(status_), "Compass calibration needs more rotation");
                    }
                }
            }
            const float sensitivity = shieldMagnetometer.getSensitivity();
            magX_ = (magnetic.raw.x - magOffsetX_) * magScaleX_ * sensitivity;
            magY_ = (magnetic.raw.y - magOffsetY_) * magScaleY_ * sensitivity;
            magZ_ = (magnetic.raw.z - magOffsetZ_) * magScaleZ_ * sensitivity;
            magUnalignedHeading_ = normalizeHeading(atan2f(magY_, magX_) * 180.0f / M_PI);
            magHeading_ = normalizeHeading(magUnalignedHeading_ + magHeadingOffset_);
        }
    }
    snapshot.hasShield = present_;
    snapshot.shieldExpanderReady = shieldExpanderReady;
    snapshot.shieldLoraReady = loraReady_;
    snapshot.shieldCodecReady = codecReady_;
    snapshot.shieldAudioReady = audioReady_;
    snapshot.audioActive = audioActive_;
    snapshot.audioFault = audioFault_ || audioRateSyncFailed;
    snapshot.shieldMicReady = micReady_;
    snapshot.micActive = micActive_;
    snapshot.micFault = micFault_;
    snapshot.shieldIrReady = irReady_;
    snapshot.irTxActive = irTxActive_;
    snapshot.irTxRunning = irTxRunning_;
    snapshot.irRxActive = irReceiveActive_;
    snapshot.irFault = irFault_;
    snapshot.irTxCount = irTxCount_;
    snapshot.shieldImuReady = imuReady_;
    snapshot.shieldMagReady = magnetometerReady_;
    snapshot.imuLevel = imuReady_ && fabsf(imuAx_) <= 0.15f && fabsf(imuAy_) <= 0.15f &&
                        fabsf(fabsf(imuAz_) - 1.0f) <= 0.20f;
    snapshot.magCalibrating = magCalibrating_;
    snapshot.magCalibrated = magCalibrated_;
    snapshot.magNorthAlignmentPending = magNorthAlignmentPending_;
    snapshot.magCalibrationProgress = magCalibrationProgress_;
    snapshot.shieldIrMatch = irMatch_;
    snapshot.shieldIrCode = irCode_;
    snapshot.irIsNec = irIsNec_;
    snapshot.irBits = irBits_;
    snprintf(snapshot.irRaw, sizeof(snapshot.irRaw), "%s", irRaw_);
    snapshot.micLevel = micLevel_;
    memcpy(snapshot.micWaveform, micWaveform_, sizeof(micWaveform_));
    snapshot.imuAx = imuAx_; snapshot.imuAy = imuAy_; snapshot.imuAz = imuAz_;
    snapshot.gyroGx = gyroGx_; snapshot.gyroGy = gyroGy_; snapshot.gyroGz = gyroGz_;
    snapshot.magX = magX_; snapshot.magY = magY_; snapshot.magZ = magZ_;
    snapshot.magHeading = magHeading_;
    snapshot.loraFrequency = loraFrequency_;
    snapshot.loraBandwidth = loraBandwidth_;
    snapshot.loraSpreadingFactor = loraSpreadingFactor_;
    snapshot.loraCodingRate = loraCodingRate_;
    snapshot.loraPower = loraPower_;
    snapshot.loraTxIntervalMs = loraTxIntervalMs_;
    const uint32_t now = millis();
    snapshot.loraTxCountdownMs = loraTxPeriodic_ && !loraTxActive_ &&
                                 (int32_t)(loraNextTxMs_ - now) > 0 ? loraNextTxMs_ - now : 0;
    snapshot.loraPreambleLength = loraPreambleLength_;
    snapshot.loraSyncWord = loraSyncWord_;
    snapshot.loraCrcEnabled = loraCrcEnabled_;
    snprintf(snapshot.loraTxMessage, sizeof(snapshot.loraTxMessage), "%s", loraTxMessage_);
    snprintf(snapshot.loraRxMessage, sizeof(snapshot.loraRxMessage), "%s", loraRxMessage_);
    snapshot.loraRssi = loraRssi_;
    snapshot.loraSnr = loraSnr_;
    snapshot.loraTxActive = loraTxActive_;
    snapshot.loraRxActive = loraRxActive_;
    snapshot.loraFault = loraFault_;
    snapshot.loraTxCount = loraTxCount_;
    snapshot.loraRxCount = loraRxCount_;
    snapshot.audioVolume = audioVolume_;
    snprintf(snapshot.shieldStatus, sizeof(snapshot.shieldStatus), "%s", status_);
}

void FactoryShield::loopAudio()
{
    if (!audioActive_ || !activeAudioGenerator) return;
    const bool running = activeAudioGenerator->loop();
    if (audioRateSyncFailed) {
        if (activeAudioGenerator->isRunning()) activeAudioGenerator->stop();
        audioActive_ = false;
        audioFault_ = true;
        setAudioAmplifierPower(false);
        setStatusText(status_, sizeof(status_), "Audio sample-rate sync failed");
        return;
    }
    if (running) return;
    activeAudioGenerator->stop();
    audioActive_ = false;
    setAudioAmplifierPower(false);
    setStatusText(status_, sizeof(status_), "Audio playback complete");
}

void FactoryShield::pollIr()
{
    if (!present_) return;

    if (irTxChannel && irTxFrameDone) {
        stopIrRmtTransmitter();
        releaseIrTransmitter();
        ++irTxCount_;
        irTxActive_ = true;
        irTxUntilMs_ = millis() + 500;
        irFault_ = false;
        setStatusText(status_, sizeof(status_), "IR TX NEC 0x%08lX sent",
                      (unsigned long)kIrTestNecCode);
    } else if (irTxChannel && millis() - irTxStartedMs_ > 500) {
        stopIrRmtTransmitter();
        releaseIrTransmitter();
        irTxActive_ = false;
        irFault_ = true;
        setStatusText(status_, sizeof(status_), "IR TX timeout");
    }

    const uint32_t now = millis();
    if (irTxRunning_ && !irTxChannel && !irTxPending_ &&
        (int32_t)(now - irTxNextMs_) >= 0) {
        irTxPending_ = true;
    }

    if (irTxPending_) {
        irTxPending_ = false;
        stopIrRmtReceiver();
        digitalWrite(kIrSelectPin, LOW);
        delay(5);
        irTxStartedMs_ = millis();
        irTxNextMs_ = irTxStartedMs_ + kIrTxIntervalMs;
        if (!startIrNecRmt(kIrTestNecCode)) {
            releaseIrTransmitter();
            irTxActive_ = false;
            irFault_ = true;
            setStatusText(status_, sizeof(status_), "IR TX failed");
            return;
        }
        irTxActive_ = true;
        irTxUntilMs_ = irTxStartedMs_ + 500;
        irFault_ = false;
        setStatusText(status_, sizeof(status_), "IR TX transmitting");
    }

    if (!irReceiveActive_) return;
    if (!irRxFrameReady) return;

    const size_t symbolCount = irRxSymbolCount;
    uint32_t decoded = 0;
    irIsNec_ = decodeNecRmt(irRxSymbols, symbolCount, decoded);
    irCode_ = irIsNec_ ? decoded : 0;
    irBits_ = irIsNec_ ? 32 : 0;
    const bool matchesFactoryCode = irIsNec_ && irCode_ == kIrTestNecCode;
    if (matchesFactoryCode) irMatch_ = !irMatch_;
    irFault_ = false;
    irRaw_[0] = '\0';
    size_t rawOffset = 0;
    const size_t rawSymbolCount = symbolCount < 12 ? symbolCount : 12;
    for (size_t i = 0; i < rawSymbolCount && rawOffset + 16 < sizeof(irRaw_); ++i) {
        rawOffset += snprintf(irRaw_ + rawOffset, sizeof(irRaw_) - rawOffset,
                              "%s0x%04X 0x%04X", i ? " " : "",
                              (unsigned)irRxSymbols[i].duration0,
                              (unsigned)irRxSymbols[i].duration1);
    }
    setStatusText(status_, sizeof(status_), irIsNec_ ? "IR RX NEC" : "IR RX non-NEC");
    irRxSymbolCount = 0;
    irRxFrameReady = false;
    if (!armIrRmtReceiver()) {
        irFault_ = true;
        irReceiveActive_ = false;
        setStatusText(status_, sizeof(status_), "IR RX restart failed");
    }
}

bool FactoryShield::runTest(uint8_t testId)
{
    if (!present_) return false;
    switch (testId) {
    case 16:
    case 17:
    {
        audioFault_ = false;
        audioRateSyncFailed = false;
        if (!audioReady_ || !audioOutput) {
            audioFault_ = true;
            setAudioAmplifierPower(false);
            return false;
        }
        if (micChannelsEnabled_) {
            i2s_channel_disable(micRxChannel);
            i2s_channel_disable(micTxChannel);
            micChannelsEnabled_ = false;
            micActive_ = false;
        }
        const bool playMp3 = testId == 17;
        if (playMp3 && !mp3Generator) mp3Generator = new (std::nothrow) AudioGeneratorMP3();
        if (!playMp3 && !toneGenerator) toneGenerator = new (std::nothrow) AudioGeneratorRTTTL();
        if (!programAudioSource) programAudioSource = new (std::nothrow) AudioFileSourcePROGMEM();
        AudioGenerator *generator = playMp3
                                        ? static_cast<AudioGenerator *>(mp3Generator)
                                        : static_cast<AudioGenerator *>(toneGenerator);
        if (!generator || !programAudioSource) {
            audioFault_ = true;
            setAudioAmplifierPower(false);
            return false;
        }
        if (activeAudioGenerator && activeAudioGenerator->isRunning()) {
            activeAudioGenerator->stop();
            audioActive_ = false;
        }
        const void *sourceData = kVanLifeMp3;
        size_t sourceLength = kVanLifeMp3Size;
        if (!playMp3) {
            sourceData = kToneBeep;
            sourceLength = sizeof(kToneBeep) - 1;
        }
        if (!programAudioSource->open(sourceData, sourceLength)) {
            audioFault_ = true;
            setAudioAmplifierPower(false);
            setStatusText(status_, sizeof(status_), "Audio source failed");
            return false;
        }
        setAudioAmplifierPower(true);
        delay(5);
        activeAudioGenerator = generator;
        audioActive_ = activeAudioGenerator->begin(programAudioSource, audioOutput);
        audioFault_ = !audioActive_;
        if (!audioActive_) {
            programAudioSource->close();
            setAudioAmplifierPower(false);
        }
        setStatusText(status_, sizeof(status_), audioActive_
                      ? (playMp3 ? "MP3 playback started" : "Beep playback started")
                      : "Audio playback failed");
        return audioActive_;
    }
    case 2: {
        micFault_ = false;
        audioRateSyncFailed = false;
        micLevel_ = 0;
        if (audioActive_ && activeAudioGenerator) {
            activeAudioGenerator->stop();
            audioActive_ = false;
        }
        setAudioAmplifierPower(false);
        const bool micRateReady = audioOutput && audioOutput->SetRate(44100);
        size_t preloaded = 0;
        esp_err_t preload = ESP_OK;
        esp_err_t txEnable = ESP_OK;
        esp_err_t rxEnable = ESP_OK;
        if (micReady_ && micRateReady && !micChannelsEnabled_) {
            static int16_t silence[1024] = {};
            preload = i2s_channel_preload_data(micTxChannel, silence, sizeof(silence), &preloaded);
            txEnable = preload == ESP_OK ? i2s_channel_enable(micTxChannel) : preload;
            rxEnable = txEnable == ESP_OK ? i2s_channel_enable(micRxChannel) : txEnable;
            micChannelsEnabled_ = txEnable == ESP_OK && rxEnable == ESP_OK;
        }
        micActive_ = micReady_ && micRateReady && micChannelsEnabled_;
        micFault_ = !micActive_;
        micLastDataMs_ = millis();
        setStatusText(status_, sizeof(status_), micActive_ ? "Microphone sampling" : "Microphone unavailable");
        return micActive_;
    }
    case 3:
        loraFault_ = false;
        if (!loraReady_) {
            loraFault_ = true;
            return false;
        }
        lora.standby();
        loraRxActive_ = false;
        loraTxCount_ = 0;
        loraTxSequence_ = 0;
        loraNextTxMs_ = millis();
        loraTxActive_ = false;
        loraTxPeriodic_ = true;
        loraIrqFlag = false;
        if (!startLoraPacket()) {
            loraTxActive_ = false;
            loraTxPeriodic_ = false;
            loraFault_ = true;
            setStatusText(status_, sizeof(status_), "LoRa TX failed");
            return false;
        }
        setStatusText(status_, sizeof(status_), "LoRa TX %s", loraTxMessage_);
        return true;
    case 4:
        loraFault_ = false;
        if (!loraReady_) {
            loraFault_ = true;
            return false;
        }
        lora.standby();
        loraTxActive_ = false;
        loraTxPeriodic_ = false;
        loraRxActive_ = true;
        loraIrqFlag = false;
        if (lora.startReceive() != RADIOLIB_ERR_NONE) {
            loraRxActive_ = false;
            loraFault_ = true;
            setStatusText(status_, sizeof(status_), "LoRa RX failed");
            return false;
        }
        loraRxMessage_[0] = '\0';
        loraRssi_ = 0;
        loraSnr_ = 0;
        setStatusText(status_, sizeof(status_), "LoRa RX listening");
        return true;
    case 5:
        irFault_ = false;
        if (!irReady_) {
            irFault_ = true;
            return false;
        }
        loraTxPeriodic_ = false;
        if (irTxRunning_) {
            irTxRunning_ = false;
            irTxPending_ = false;
            stopIrRmtTransmitter();
            releaseIrTransmitter();
            irTxActive_ = false;
            irTxStartedMs_ = 0;
            irTxNextMs_ = 0;
            setStatusText(status_, sizeof(status_), "IR TX stopped");
            return true;
        }
        if (irReceiveActive_) irReceiveActive_ = false;
        stopIrRmtReceiver();
        digitalWrite(kIrSelectPin, LOW);
        irTxRunning_ = true;
        irTxNextMs_ = millis();
        irTxPending_ = true;
        setStatusText(status_, sizeof(status_), "IR TX started: 1 frame/s");
        return true;
    case 6:
        irFault_ = false;
        if (!irReady_) {
            irFault_ = true;
            return false;
        }
        loraTxPeriodic_ = false;
        irTxRunning_ = false;
        irTxPending_ = false;
        stopIrRmtTransmitter();
        irTxActive_ = false;
        irReceiveActive_ = false;
        stopIrRmtReceiver();
        digitalWrite(kIrSelectPin, HIGH);
        delay(5);
        irRaw_[0] = '\0';
        irCode_ = 0;
        irMatch_ = false;
        irIsNec_ = false;
        irBits_ = 0;
        irReceiveActive_ = startIrRmtReceiver();
        if (!irReceiveActive_) {
            irFault_ = true;
            setStatusText(status_, sizeof(status_), "IR RX unavailable");
            return false;
        }
        setStatusText(status_, sizeof(status_), "IR RX listening");
        return true;
    case 7:
        loraTxPeriodic_ = false;
        setStatusText(status_, sizeof(status_), sensorReady_ ? "Sensors sampling" : "Sensors unavailable");
        return sensorReady_;
    default: return false;
    }
}

bool FactoryShield::playSdMp3(const char *path)
{
    audioFault_ = false;
    audioRateSyncFailed = false;
    if (!present_ || !audioReady_ || !audioOutput || !path || !path[0]) {
        audioFault_ = true;
        setAudioAmplifierPower(false);
        setStatusText(status_, sizeof(status_), "SD audio unavailable");
        return false;
    }
    if (micChannelsEnabled_) {
        i2s_channel_disable(micRxChannel);
        i2s_channel_disable(micTxChannel);
        micChannelsEnabled_ = false;
        micActive_ = false;
    }
    if (!mp3Generator) mp3Generator = new (std::nothrow) AudioGeneratorMP3();
    if (!sdAudioSource) sdAudioSource = new (std::nothrow) AudioFileSourceSD();
    if (!mp3Generator || !sdAudioSource) {
        audioFault_ = true;
        setAudioAmplifierPower(false);
        setStatusText(status_, sizeof(status_), "SD audio allocation failed");
        return false;
    }
    if (activeAudioGenerator && activeAudioGenerator->isRunning()) {
        activeAudioGenerator->stop();
        audioActive_ = false;
    }
    if (!sdAudioSource->open(path)) {
        audioFault_ = true;
        setAudioAmplifierPower(false);
        setStatusText(status_, sizeof(status_), "MP3 file open failed");
        return false;
    }
    setAudioAmplifierPower(true);
    delay(5);
    activeAudioGenerator = mp3Generator;
    audioActive_ = activeAudioGenerator->begin(sdAudioSource, audioOutput);
    audioFault_ = !audioActive_;
    if (!audioActive_) {
        sdAudioSource->close();
        setAudioAmplifierPower(false);
    }
    setStatusText(status_, sizeof(status_), audioActive_ ? "SD MP3 playback started" : "MP3 decode failed");
    return audioActive_;
}

bool FactoryShield::configureLora(uint8_t setting, int16_t value)
{
    if (!loraReady_) return false;
    switch (setting) {
    case 1:
        loraPower_ = constrain(value, -9, 22);
        lora.setOutputPower(loraPower_);
        break;
    case 2: {
        static const float bandwidths[] = {7.8f, 15.6f, 31.25f, 62.5f, 125.0f, 250.0f, 500.0f};
        const uint8_t index = constrain(value, 0, 6);
        loraBandwidth_ = bandwidths[index];
        lora.setBandwidth(loraBandwidth_);
        break;
    }
    case 3:
        loraSpreadingFactor_ = constrain(value, 5, 12);
        lora.setSpreadingFactor(loraSpreadingFactor_);
        break;
    case 4:
    {
        static const float frequencies[] = {433.0f, 470.0f, 868.0f, 915.0f, 920.0f, 923.0f};
        const uint8_t index = constrain(value, 0, 5);
        loraFrequency_ = frequencies[index];
        lora.setFrequency(loraFrequency_);
        break;
    }
    case 5:
        loraCodingRate_ = constrain(value, 5, 8);
        lora.setCodingRate(loraCodingRate_);
        break;
    case 6:
        loraTxIntervalMs_ = constrain(value, 1, 100) * 100UL;
        break;
    case 7:
        loraPreambleLength_ = constrain(value, 6, 64);
        lora.setPreambleLength(loraPreambleLength_);
        break;
    case 8:
        loraSyncWord_ = constrain(value, 0, 255);
        lora.setSyncWord(loraSyncWord_);
        break;
    case 9:
        loraCrcEnabled_ = value != 0;
        lora.setCRC(loraCrcEnabled_ ? 2 : 0);
        break;
    default: return false;
    }
    setStatusText(status_, sizeof(status_), "LoRa parameters updated");
    return true;
}

bool FactoryShield::startMagCalibration()
{
    if (!magnetometerReady_) return false;
    if (magCalibrated_ && magNorthAlignmentPending_) {
        magHeadingOffset_ = normalizeHeading(-magUnalignedHeading_);
        magHeading_ = normalizeHeading(magUnalignedHeading_ + magHeadingOffset_);
        magNorthAlignmentPending_ = false;
        setStatusText(status_, sizeof(status_), "Compass aligned to North");
        return true;
    }
    shieldMagnetometer.setOffset(0, 0, 0);
    magOffsetX_ = magOffsetY_ = magOffsetZ_ = 0;
    magScaleX_ = magScaleY_ = magScaleZ_ = 1.0f;
    magHeadingOffset_ = 0.0f;
    magMinX_ = magMinY_ = magMinZ_ = INT16_MAX;
    magMaxX_ = magMaxY_ = magMaxZ_ = INT16_MIN;
    magCalibrationStartedMs_ = millis();
    magCalibrationProgress_ = 0;
    magCalibrated_ = false;
    magNorthAlignmentPending_ = false;
    magCalibrating_ = true;
    setStatusText(status_, sizeof(status_), "Rotate the shield through every axis");
    return true;
}

bool FactoryShield::setAudioVolume(uint8_t value)
{
    if (!audioReady_) return false;
    audioVolume_ = constrain(value, 0, FACTORY_MAX_AUDIO_VOLUME);
    return audioBoard.setVolume(audioVolume_);
}

void FactoryShield::stopActiveTests()
{
    if (audioActive_ && activeAudioGenerator) {
        activeAudioGenerator->stop();
        audioActive_ = false;
    }
    setAudioAmplifierPower(false);
    if (micChannelsEnabled_) {
        i2s_channel_disable(micRxChannel);
        i2s_channel_disable(micTxChannel);
        micChannelsEnabled_ = false;
    }
    micActive_ = false;
    loraTxPeriodic_ = false;
    irTxPending_ = false;
    irTxActive_ = false;
    irTxRunning_ = false;
    irTxStartedMs_ = 0;
    irTxNextMs_ = 0;
    loraTxActive_ = false;
    loraRxActive_ = false;
    if (loraReady_) lora.standby();
    if (irReady_) stopIrRmtReceiver();
    stopIrRmtTransmitter();
    irReceiveActive_ = false;
    if (irReady_) releaseIrTransmitter();
}
