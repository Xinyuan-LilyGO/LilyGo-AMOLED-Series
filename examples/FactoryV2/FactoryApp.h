#pragma once

#include <Adafruit_NeoPixel.h>
#include <SD.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include "ping/ping_sock.h"
#include "FactoryData.h"
#include "FactoryExtension.h"
#include "FactoryShield.h"
#include "FactoryUi.h"

class FactoryApp {
public:
    bool begin();
    void loop();
    bool addExtension(FactoryExtension &extension);
    void setBleConnectionState(bool connected);

private:
    static void timerEvent(lv_timer_t *timer);
    static void homeButtonAction(void *context);
    static void displayTestAction(void *context);
    static void rotateDisplayAction(void *context);
    static void brightnessAction(void *context, uint8_t value);
    static void mountSdAction(void *context);
    static void sdMusicScanAction(void *context);
    static void sdMusicPlayAction(void *context, uint8_t track);
    static void pixelTestAction(void *context);
    static void shieldTestAction(void *context, uint8_t testId);
    static void powerOffAction(void *context);
    static void shieldConfigAction(void *context, uint8_t setting, int16_t value);
    static void shieldAudioVolumeAction(void *context, uint8_t value);
    static void shieldMagCalibrateAction(void *context);
    static void shieldStopAction(void *context);
    static void extensionAction(void *context, uint8_t slot, uint8_t action);
    static void powerOffConfirmedAction(void *context);
    static void wifiScanAction(void *context);
    static void wifiConnectAction(void *context, uint8_t network);
    static void wifiPingAction(void *context);
    static void bleSetActiveAction(void *context, bool active);
    static void wifiPingSuccess(esp_ping_handle_t handle, void *args);
    static void wifiPingTimeout(esp_ping_handle_t handle, void *args);
    static void wifiPingEnd(esp_ping_handle_t handle, void *args);
    void updateSnapshot();
    void updateWifi();
    void updateClock();
    void startNetworkTimeSync();
    void setBleActive(bool active);
    void readIdentity();
    void pollBootButton();
    void runSdTestStep();
    void startSdMusicScan();
    void runSdMusicScanStep();
    void finishSdMusicScan(const char *status);

    LilyGo_Class amoled_;
    const BoardsConfigure_t *board_ = nullptr;
    FactorySnapshot snapshot_;
    FactoryShield shield_;
    FactoryUi ui_;
    Adafruit_NeoPixel *pixels_ = nullptr;
    uint8_t displayTestColor_ = 0;
    bool uiRebuildPending_ = false;
    bool bootButtonLatched_ = false;
    bool bootButtonReleasePending_ = false;
    uint32_t bootButtonReleaseStartedMs_ = 0;
    char lastShieldStatus_[64] = {};
    bool sdTestActive_ = false;
    uint8_t sdTestPhase_ = 0;
    uint32_t sdTestStartMs_ = 0;
    uint32_t sdWriteStartMs_ = 0;
    uint32_t sdReadStartMs_ = 0;
    uint32_t sdTestBytes_ = 0;
    uint32_t sdTestTargetBytes_ = 256 * 1024;
    File sdTestFile_;
    uint8_t sdTestBuffer_[4096] = {};
    bool sdMusicScanActive_ = false;
    bool sdMusicPlaybackActive_ = false;
    uint8_t sdMusicScanPhase_ = 0;
    File sdMusicDirectory_;
    FactoryExtension *extensions_[FACTORY_MAX_EXTENSIONS] = {};
    uint8_t extensionCount_ = 0;
    bool wifiStarted_ = false;
    bool wifiScanPending_ = false;
    bool wifiPingPending_ = false;
    volatile bool wifiPingDone_ = false;
    volatile uint16_t wifiPingReplies_ = 0;
    volatile uint32_t wifiPingLastMs_ = 0;
    esp_ping_handle_t wifiPingHandle_ = nullptr;
    uint32_t wifiConnectStartedMs_ = 0;
    uint32_t lastClockUpdateMs_ = 0;
    bool networkTimeSyncStarted_ = false;
    bool networkTimeSynced_ = false;
};
