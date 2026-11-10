#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <esp_err.h>
#include "FactoryData.h"

class FactoryShield {
public:
    bool begin(TwoWire &wire, int sda, int scl);
    void loopAudio();
    bool audioActive() const { return audioActive_; }
    void update(FactorySnapshot &snapshot);
    bool present() const { return present_; }
    bool runTest(uint8_t testId);
    bool playSdMp3(const char *path);
    void pollIr();
    bool configureLora(uint8_t setting, int16_t value);
    bool setAudioVolume(uint8_t value);
    bool startMagCalibration();
    void stopActiveTests();
    const char *lastStatus() const { return status_; }

private:
    bool probe(TwoWire &wire, uint8_t address);
    bool startLoraPacket();
    bool present_ = false;
    bool loraReady_ = false;
    bool codecReady_ = false;
    bool audioReady_ = false;
    bool irReady_ = false;
    bool sensorReady_ = false;
    bool imuReady_ = false;
    bool magnetometerReady_ = false;
    bool micReady_ = false;
    bool micChannelsEnabled_ = false;
    bool micActive_ = false;
    bool audioActive_ = false;
    bool audioFault_ = false;
    bool micFault_ = false;
    bool loraTxActive_ = false;
    bool loraRxActive_ = false;
    bool loraFault_ = false;
    uint32_t loraTxCount_ = 0;
    uint32_t loraTxSequence_ = 0;
    uint32_t loraRxCount_ = 0;
    bool irReceiveActive_ = false;
    bool irTxPending_ = false;
    bool irTxActive_ = false;
    bool irTxRunning_ = false;
    bool irFault_ = false;
    uint32_t irTxStartedMs_ = 0;
    uint32_t irTxUntilMs_ = 0;
    uint32_t irTxNextMs_ = 0;
    uint32_t irTxCount_ = 0;
    bool irMatch_ = false;
    bool irIsNec_ = false;
    uint8_t irBits_ = 0;
    uint32_t irCode_ = 0;
    char irRaw_[256] = {};
    int micLevel_ = 0;
    uint32_t micLastDataMs_ = 0;
    int16_t micWaveform_[32] = {};
    float imuAx_ = 0;
    float imuAy_ = 0;
    float imuAz_ = 0;
    float gyroGx_ = 0;
    float gyroGy_ = 0;
    float gyroGz_ = 0;
    float magX_ = 0;
    float magY_ = 0;
    float magZ_ = 0;
    float magHeading_ = 0;
    float magUnalignedHeading_ = 0;
    float magHeadingOffset_ = 0;
    float magScaleX_ = 1;
    float magScaleY_ = 1;
    float magScaleZ_ = 1;
    int16_t magOffsetX_ = 0;
    int16_t magOffsetY_ = 0;
    int16_t magOffsetZ_ = 0;
    bool magCalibrating_ = false;
    bool magCalibrated_ = false;
    bool magNorthAlignmentPending_ = false;
    uint8_t magCalibrationProgress_ = 0;
    uint32_t magCalibrationStartedMs_ = 0;
    int16_t magMinX_ = 0;
    int16_t magMinY_ = 0;
    int16_t magMinZ_ = 0;
    int16_t magMaxX_ = 0;
    int16_t magMaxY_ = 0;
    int16_t magMaxZ_ = 0;
    float loraFrequency_ = 868.0f;
    float loraBandwidth_ = 125.0f;
    uint8_t loraSpreadingFactor_ = 7;
    uint8_t loraCodingRate_ = 5;
    int8_t loraPower_ = 22;
    uint32_t loraTxIntervalMs_ = 1000;
    uint8_t loraPreambleLength_ = 8;
    uint8_t loraSyncWord_ = 0x12;
    bool loraCrcEnabled_ = true;
    bool loraTxPeriodic_ = false;
    uint32_t loraNextTxMs_ = 0;
    char loraTxMessage_[64] = {};
    char loraRxMessage_[96] = {};
    int16_t loraRssi_ = 0;
    float loraSnr_ = 0;
    uint8_t audioVolume_ = FACTORY_DEFAULT_AUDIO_VOLUME;
    char status_[64] = {};
};
