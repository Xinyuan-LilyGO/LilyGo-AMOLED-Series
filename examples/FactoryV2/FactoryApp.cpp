#include "FactoryApp.h"

#include <Arduino.h>
#include <SD.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include <BLEServer.h>
#include "ping/ping_sock.h"
#include <esp_mac.h>
#include <esp_sntp.h>
#include <esp_sleep.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif
#ifndef WIFI_SSID2
#define WIFI_SSID2 ""
#endif
#ifndef WIFI_PASSWORD2
#define WIFI_PASSWORD2 ""
#endif
#ifndef FACTORY_TIMEZONE
#define FACTORY_TIMEZONE "CST-8"
#endif
#ifndef FACTORY_NTP_SERVER1
#define FACTORY_NTP_SERVER1 "pool.ntp.org"
#endif
#ifndef FACTORY_NTP_SERVER2
#define FACTORY_NTP_SERVER2 "time.nist.gov"
#endif

namespace {
const lv_color_t kDisplayColors[] = {
    lv_color_make(255, 0, 0), lv_color_make(0, 255, 0), lv_color_make(0, 0, 255),
    lv_color_make(255, 255, 255), lv_color_black(),
};
constexpr uint16_t kShieldChargeCurrentMa = 200;
constexpr uint32_t kBootButtonReleaseDebounceMs = 20;

bool isMp3Path(const char *path)
{
    if (!path) return false;
    const size_t length = strlen(path);
    return length > 4 && strcasecmp(path + length - 4, ".mp3") == 0;
}

FactoryApp *gFactoryApp = nullptr;

class FactoryBleServerCallbacks final : public BLEServerCallbacks {
public:
    void onConnect(BLEServer *) override
    {
        if (gFactoryApp) gFactoryApp->setBleConnectionState(true);
    }

    void onDisconnect(BLEServer *) override
    {
        if (gFactoryApp) gFactoryApp->setBleConnectionState(false);
    }
};
}

bool FactoryApp::begin()
{
    gFactoryApp = this;
    if (!amoled_.begin()) {
        while (true) delay(1000);
    }
    board_ = amoled_.getBoardsConfigure();
    for (uint8_t i = 0; i < board_->buttonNum; ++i) {
        pinMode(board_->pButtons[i], INPUT_PULLUP);
    }
    bootButtonLatched_ = board_->buttonNum > 0 && digitalRead(board_->pButtons[0]) == LOW;
    bootButtonReleasePending_ = false;
    bootButtonReleaseStartedMs_ = 0;
    readIdentity();
    if (snapshot_.hasPmic) {
        // The shared AMOLED board wrapper leaves SY6970/BQ25896 charging
        // disabled after detection. FactoryV2 must enable it before reading
        // the live charger state.
        amoled_.enableCharge();
    }
    const int i2cSda = board_->touch ? board_->touch->sda :
                       (board_->pmu ? board_->pmu->sda :
                        (board_->sensor ? board_->sensor->sda : -1));
    const int i2cScl = board_->touch ? board_->touch->scl :
                       (board_->pmu ? board_->pmu->scl :
                        (board_->sensor ? board_->sensor->scl : -1));
    const bool shieldDetected = shield_.begin(Wire, i2cSda, i2cScl);
    if (shieldDetected && amoled_.height() > amoled_.width()) {
        amoled_.setRotation((amoled_.getRotation() + 1) % 4);
        snapshot_.width = amoled_.width();
        snapshot_.height = amoled_.height();
    }
    if (shieldDetected && snapshot_.hasPmic) {
        PmicChargerBase *charger = amoled_.pmic()->getCharger();
        if (charger) charger->setFastChargeCurrent(kShieldChargeCurrentMa);
    }
    if (shieldDetected) {
        shield_.update(snapshot_);
    }
    setenv("TZ", FACTORY_TIMEZONE, 1);
    tzset();
    if (snapshot_.hasRtc) {
        RTC_DateTime rtc = amoled_.rtc()->getDateTime();
        if (rtc.getYear() >= 2020) {
            amoled_.rtc()->hwClockRead();
        }
    }
    for (uint8_t i = 0; i < extensionCount_; ++i) {
        extensions_[i]->begin(amoled_, board_);
        FactoryExtensionStatus &status = snapshot_.extensions[i];
        snprintf(status.id, sizeof(status.id), "%s", extensions_[i]->id());
        snprintf(status.title, sizeof(status.title), "%s", extensions_[i]->title());
        snprintf(status.icon, sizeof(status.icon), "%s", extensions_[i]->icon());
        status.testable = true;
    }
    snapshot_.extensionCount = extensionCount_;
    snprintf(lastShieldStatus_, sizeof(lastShieldStatus_), "%s", snapshot_.shieldStatus);
    beginLvglHelper(amoled_);

    if (board_->pixelsPins != -1) {
        pixels_ = new (std::nothrow) Adafruit_NeoPixel(BOARD_PIXELS_NUM, board_->pixelsPins, NEO_GRB + NEO_KHZ800);
        if (pixels_) {
            pixels_->begin();
            pixels_->clear();
            pixels_->show();
        }
    }

    FactoryActions actions;
    actions.context = this;
    actions.displayTest = displayTestAction;
    actions.rotateDisplay = rotateDisplayAction;
    actions.setBrightness = brightnessAction;
    actions.mountSd = mountSdAction;
    actions.sdMusicScan = sdMusicScanAction;
    actions.sdMusicPlay = sdMusicPlayAction;
    actions.pixelTest = pixelTestAction;
    actions.shieldTest = shieldTestAction;
    actions.shieldAudioVolume = shieldAudioVolumeAction;
    actions.shieldMagCalibrate = shieldMagCalibrateAction;
    actions.shieldStop = shieldStopAction;
    actions.powerOff = powerOffAction;
    actions.powerOffConfirmed = powerOffConfirmedAction;
    actions.shieldConfig = shieldConfigAction;
    actions.extensionAction = extensionAction;
    actions.wifiScan = wifiScanAction;
    actions.wifiConnect = wifiConnectAction;
    actions.wifiPing = wifiPingAction;
    actions.bleSetActive = bleSetActiveAction;
    ui_.begin(snapshot_, actions);
    if (TouchDrvInterface *touch = amoled_.touch()) {
        touch->setHomeButtonCallback(homeButtonAction, this);
    }
    // Keep the shield service cadence high enough to sustain the microphone
    // I2S TX clock and refresh live test indicators without blocking LVGL.
    lv_timer_create(timerEvent, 15, this);
    updateSnapshot();
    return true;
}

void FactoryApp::loop()
{
    pollBootButton();
    if (shield_.present()) {
        shield_.loopAudio();
        shield_.pollIr();
    }
    lv_timer_handler();
    pollBootButton();
    if (shield_.present() && shield_.audioActive()) {
        // Refill immediately after a potentially expensive display refresh.
        shield_.loopAudio();
        delay(1);
    } else {
        delay(5);
    }
}

bool FactoryApp::addExtension(FactoryExtension &extension)
{
    if (extensionCount_ >= FACTORY_MAX_EXTENSIONS) return false;
    extensions_[extensionCount_++] = &extension;
    return true;
}

void FactoryApp::timerEvent(lv_timer_t *timer)
{
    FactoryApp *app = static_cast<FactoryApp *>(lv_timer_get_user_data(timer));
    if (app) app->updateSnapshot();
}

void FactoryApp::pollBootButton()
{
    if (!board_ || board_->buttonNum == 0) return;
    const uint32_t now = millis();
    const bool pressed = digitalRead(board_->pButtons[0]) == LOW;
    if (pressed) {
        bootButtonReleasePending_ = false;
        if (bootButtonLatched_) return;
        bootButtonLatched_ = true;
        ui_.goHome();
        return;
    }
    if (!bootButtonLatched_) return;
    if (!bootButtonReleasePending_) {
        bootButtonReleasePending_ = true;
        bootButtonReleaseStartedMs_ = now;
        return;
    }
    if (now - bootButtonReleaseStartedMs_ >= kBootButtonReleaseDebounceMs) {
        bootButtonLatched_ = false;
        bootButtonReleasePending_ = false;
    }
}

void FactoryApp::homeButtonAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (app) app->ui_.goHome();
}

void FactoryApp::rotateDisplayAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    const uint8_t rotation = (app->amoled_.getRotation() + 1) % 4;
    app->amoled_.setRotation(rotation);
    if (lv_display_t *display = lv_display_get_default()) {
        lv_display_set_resolution(display, app->amoled_.width(), app->amoled_.height());
    }
    app->snapshot_.width = app->amoled_.width();
    app->snapshot_.height = app->amoled_.height();
    app->uiRebuildPending_ = true;
}

void FactoryApp::readIdentity()
{
    snprintf(snapshot_.boardName, sizeof(snapshot_.boardName), "%s", amoled_.getName());
    snapshot_.width = amoled_.width();
    snapshot_.height = amoled_.height();
    uint8_t mac[6] = {};
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(snapshot_.mac, sizeof(snapshot_.mac), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    snapshot_.hasTouch = amoled_.hasTouch();
    snapshot_.hasPmic = amoled_.pmic() != nullptr;
    if (snapshot_.hasPmic) {
        snprintf(snapshot_.pmicName, sizeof(snapshot_.pmicName), "%s", amoled_.pmic()->getChipName());
    } else {
        snprintf(snapshot_.pmicName, sizeof(snapshot_.pmicName), "N/A");
    }
    snapshot_.hasRtc = amoled_.hasRTC();
    snapshot_.hasLightSensor = amoled_.lightSensor() != nullptr;
    snapshot_.hasSd = board_->sd != nullptr;
    snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Open SD Music to scan");
    snapshot_.hasPixels = board_->pixelsPins != -1;
    snapshot_.hasWifi = true;
    snapshot_.hasBle = true;
    snprintf(snapshot_.wifiMac, sizeof(snapshot_.wifiMac), "%s", snapshot_.mac);
    snprintf(snapshot_.wifiStatus, sizeof(snapshot_.wifiStatus), "Open Wi-Fi to scan");
    snapshot_.wifiPingStatus[0] = '\0';
    snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "Ready to advertise");
    snapshot_.brightness = amoled_.getBrightness();
}

void FactoryApp::updateSnapshot()
{
    if (!board_) return;
    runSdTestStep();
    runSdMusicScanStep();
    updateWifi();
    updateClock();
    snapshot_.brightness = amoled_.getBrightness();
    snapshot_.freeHeap = ESP.getFreeHeap();
    snapshot_.freePsram = ESP.getFreePsram();
    snapshot_.touched = false;
    if (snapshot_.hasTouch) {
        snapshot_.touched = amoled_.getPoint(&snapshot_.touchX, &snapshot_.touchY, 1) != 0;
    }
    snapshot_.coreTemperature = amoled_.readCoreTemp();
    snapshot_.lightLux = snapshot_.hasLightSensor ? amoled_.lightSensor()->getLux() : 0;

    if (snapshot_.hasPmic) {
        snapshot_.batteryMillivolts = amoled_.getBattVoltage();
        snapshot_.vbusMillivolts = amoled_.getVbusVoltage();
        PmicChargerBase *charger = amoled_.pmic()->getCharger();
        const PmicChargerBase::Status chargerStatus = charger ? charger->getStatus() : PmicChargerBase::Status{};
        snapshot_.charging = chargerStatus.charging;
        snapshot_.vbusPresent = chargerStatus.vbusPresent;
        if (chargerStatus.charging) {
            snprintf(snapshot_.chargeStatus, sizeof(snapshot_.chargeStatus), "CHARGING");
        } else if (chargerStatus.chargeDone) {
            snprintf(snapshot_.chargeStatus, sizeof(snapshot_.chargeStatus), "FULL");
        } else {
            snprintf(snapshot_.chargeStatus, sizeof(snapshot_.chargeStatus), "IDLE");
        }
    } else {
        snapshot_.batteryMillivolts = board_->adcPins == -1 ? 0 : amoled_.getBattVoltage();
        snapshot_.vbusMillivolts = 0;
        snapshot_.charging = false;
        snapshot_.vbusPresent = false;
        snprintf(snapshot_.chargeStatus, sizeof(snapshot_.chargeStatus), "N/A");
    }
    snapshot_.sdReady = snapshot_.hasSd && SD.cardType() != CARD_NONE;
    snapshot_.sdTesting = sdTestActive_;
    snprintf(snapshot_.sd, sizeof(snapshot_.sd), "SD: %s", snapshot_.hasSd ? (snapshot_.sdReady ? "ready" : "not mounted") : "N/A");
    if (shield_.present()) {
        shield_.update(snapshot_);
    }
    snapshot_.sdMusicPlaying = sdMusicPlaybackActive_ && snapshot_.audioActive;
    if (sdMusicPlaybackActive_ && !snapshot_.audioActive) {
        sdMusicPlaybackActive_ = false;
        snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Playback complete");
    }
    if (strcmp(lastShieldStatus_, snapshot_.shieldStatus) != 0) {
        snprintf(lastShieldStatus_, sizeof(lastShieldStatus_), "%s", snapshot_.shieldStatus);
        if (snapshot_.shieldStatus[0]) ui_.showTestMessage(snapshot_.shieldStatus);
    }
    for (uint8_t i = 0; i < extensionCount_; ++i) {
        FactoryExtensionStatus &status = snapshot_.extensions[i];
        char id[sizeof(status.id)];
        char title[sizeof(status.title)];
        char icon[sizeof(status.icon)];
        snprintf(id, sizeof(id), "%s", status.id);
        snprintf(title, sizeof(title), "%s", status.title);
        snprintf(icon, sizeof(icon), "%s", status.icon);
        const bool testable = status.testable;
        extensions_[i]->update(status);
        snprintf(status.id, sizeof(status.id), "%s", id);
        snprintf(status.title, sizeof(status.title), "%s", title);
        snprintf(status.icon, sizeof(status.icon), "%s", icon);
        status.testable = testable;
    }
    if (uiRebuildPending_) {
        ui_.rebuild(snapshot_);
        uiRebuildPending_ = false;
    }
    ui_.update(snapshot_);
}

void FactoryApp::displayTestAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    app->ui_.showColorTest(kDisplayColors[app->displayTestColor_]);
    app->displayTestColor_ = (app->displayTestColor_ + 1) % (sizeof(kDisplayColors) / sizeof(kDisplayColors[0]));
    app->ui_.showTestMessage("Screen color test active");
}

void FactoryApp::brightnessAction(void *context, uint8_t value)
{
    static_cast<FactoryApp *>(context)->amoled_.setBrightness(value);
}

void FactoryApp::mountSdAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || !app->snapshot_.hasSd) return;
    app->sdTestActive_ = true;
    app->sdTestPhase_ = 0;
    app->sdTestBytes_ = 0;
    app->snapshot_.sdTestProgress = 0;
    snprintf(app->snapshot_.sdTestStatus, sizeof(app->snapshot_.sdTestStatus), "Mounting SD card...");
    app->ui_.showTestMessage(app->snapshot_.sdTestStatus);
}

void FactoryApp::sdMusicScanAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (app) app->startSdMusicScan();
}

void FactoryApp::sdMusicPlayAction(void *context, uint8_t track)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || track >= app->snapshot_.sdMusicTrackCount) return;
    if (app->sdMusicScanActive_) {
        if (app->sdMusicDirectory_) app->sdMusicDirectory_.close();
        app->sdMusicScanActive_ = false;
        app->snapshot_.sdMusicScanning = false;
    }
    const FactorySdMusicTrack &selected = app->snapshot_.sdMusicTracks[track];
    snprintf(app->snapshot_.sdMusicCurrentPath, sizeof(app->snapshot_.sdMusicCurrentPath),
             "%s", selected.path);
    app->sdMusicPlaybackActive_ = app->shield_.playSdMp3(selected.path);
    snprintf(app->snapshot_.sdMusicStatus, sizeof(app->snapshot_.sdMusicStatus),
             app->sdMusicPlaybackActive_ ? "Playing %s" : "Unable to play %s", selected.name);
}

void FactoryApp::shieldConfigAction(void *context, uint8_t setting, int16_t value)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || !app->shield_.configureLora(setting, value)) return;
    app->updateSnapshot();
}

void FactoryApp::shieldAudioVolumeAction(void *context, uint8_t value)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (app) app->shield_.setAudioVolume(value);
}

void FactoryApp::shieldMagCalibrateAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    if (!app->shield_.startMagCalibration()) {
        app->ui_.showTestMessage("Magnetometer unavailable");
        return;
    }
    app->updateSnapshot();
}

void FactoryApp::shieldStopAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    app->shield_.stopActiveTests();
    if (app->sdMusicScanActive_) {
        if (app->sdMusicDirectory_) app->sdMusicDirectory_.close();
        app->sdMusicScanActive_ = false;
        app->snapshot_.sdMusicScanning = false;
        app->snapshot_.sdMusicGeneration++;
    }
    if (app->sdMusicPlaybackActive_) {
        app->sdMusicPlaybackActive_ = false;
        app->snapshot_.sdMusicPlaying = false;
        snprintf(app->snapshot_.sdMusicStatus, sizeof(app->snapshot_.sdMusicStatus), "Playback stopped");
    }
}

void FactoryApp::wifiScanAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    WiFi.mode(WIFI_STA);
    app->wifiStarted_ = true;
    app->wifiScanPending_ = true;
    app->snapshot_.wifiScanning = true;
    app->snapshot_.wifiConnecting = false;
    app->snapshot_.wifiConnected = WiFi.status() == WL_CONNECTED;
    app->snapshot_.wifiNetworkCount = 0;
    app->snapshot_.wifiScanGeneration++;
    snprintf(app->snapshot_.wifiStatus, sizeof(app->snapshot_.wifiStatus), "Scanning nearby networks...");
    WiFi.scanDelete();
    WiFi.scanNetworks(true, true);
}

void FactoryApp::wifiConnectAction(void *context, uint8_t network)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || network >= app->snapshot_.wifiNetworkCount) return;
    const char *ssid = app->snapshot_.wifiNetworks[network].ssid;
    const char *password = nullptr;
    if (WIFI_SSID[0] && strcmp(ssid, WIFI_SSID) == 0) password = WIFI_PASSWORD;
    else if (WIFI_SSID2[0] && strcmp(ssid, WIFI_SSID2) == 0) password = WIFI_PASSWORD2;
    if (!password) {
        snprintf(app->snapshot_.wifiStatus, sizeof(app->snapshot_.wifiStatus),
                 "No configured password for %s", ssid);
        app->snapshot_.wifiConnecting = false;
        return;
    }
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    WiFi.begin(ssid, password);
    app->wifiStarted_ = true;
    app->wifiConnectStartedMs_ = millis();
    app->snapshot_.wifiIp[0] = '\0';
    app->snapshot_.wifiConnecting = true;
    app->snapshot_.wifiConnected = false;
    snprintf(app->snapshot_.wifiStatus, sizeof(app->snapshot_.wifiStatus), "Connecting to %s...", ssid);
}

void FactoryApp::wifiPingAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    if (WiFi.status() != WL_CONNECTED) {
        snprintf(app->snapshot_.wifiPingStatus, sizeof(app->snapshot_.wifiPingStatus), "Connect to Wi-Fi first");
        app->wifiPingPending_ = false;
        return;
    }
    if (app->wifiPingHandle_) return;
    IPAddress address;
    if (!WiFi.hostByName("baidu.com", address)) {
        snprintf(app->snapshot_.wifiPingStatus, sizeof(app->snapshot_.wifiPingStatus), "baidu.com DNS failed");
        app->snapshot_.wifiPingOk = false;
        return;
    }
    ip_addr_t target = {};
    target.type = IPADDR_TYPE_V4;
    target.u_addr.ip4.addr = (uint32_t)address;
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.target_addr = target;
    config.count = 3;
    config.interval_ms = 250;
    config.timeout_ms = 700;
    esp_ping_callbacks_t callbacks = {};
    callbacks.cb_args = app;
    callbacks.on_ping_success = wifiPingSuccess;
    callbacks.on_ping_timeout = wifiPingTimeout;
    callbacks.on_ping_end = wifiPingEnd;
    app->wifiPingDone_ = false;
    app->wifiPingReplies_ = 0;
    app->wifiPingLastMs_ = 0;
    if (esp_ping_new_session(&config, &callbacks, &app->wifiPingHandle_) != ESP_OK ||
        esp_ping_start(app->wifiPingHandle_) != ESP_OK) {
        if (app->wifiPingHandle_) esp_ping_delete_session(app->wifiPingHandle_);
        app->wifiPingHandle_ = nullptr;
        snprintf(app->snapshot_.wifiPingStatus, sizeof(app->snapshot_.wifiPingStatus), "Ping session failed");
        app->snapshot_.wifiPingOk = false;
        return;
    }
    app->wifiPingPending_ = true;
    app->snapshot_.wifiPingRunning = true;
    snprintf(app->snapshot_.wifiPingStatus, sizeof(app->snapshot_.wifiPingStatus), "Pinging baidu.com...");
}

void FactoryApp::wifiPingSuccess(esp_ping_handle_t handle, void *args)
{
    FactoryApp *app = static_cast<FactoryApp *>(args);
    if (!app) return;
    uint32_t elapsed = 0;
    esp_ping_get_profile(handle, ESP_PING_PROF_TIMEGAP, &elapsed, sizeof(elapsed));
    app->wifiPingReplies_ = (uint16_t)(app->wifiPingReplies_ + 1);
    app->wifiPingLastMs_ = elapsed;
}

void FactoryApp::wifiPingTimeout(esp_ping_handle_t, void *)
{
}

void FactoryApp::wifiPingEnd(esp_ping_handle_t, void *args)
{
    FactoryApp *app = static_cast<FactoryApp *>(args);
    if (app) app->wifiPingDone_ = true;
}

void FactoryApp::bleSetActiveAction(void *context, bool active)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (app) app->setBleActive(active);
}

void FactoryApp::setBleActive(bool active)
{
    if (!snapshot_.hasBle) return;
    if (active && !snapshot_.bleActive) {
        if (!BLEDevice::getInitialized()) {
            BLEDevice::init("FactoryV2 BLE");
            BLEServer *server = BLEDevice::createServer();
            if (server) server->setCallbacks(new FactoryBleServerCallbacks());
        }
        BLEAdvertising *advertising = BLEDevice::getAdvertising();
        if (advertising) {
            advertising->setScanResponse(true);
            BLEDevice::startAdvertising();
            snapshot_.bleActive = true;
            snapshot_.bleConnected = false;
            snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "Advertising: FactoryV2 BLE");
        } else {
            snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "BLE advertising unavailable");
        }
    } else if (!active && snapshot_.bleActive) {
        BLEDevice::stopAdvertising();
        snapshot_.bleActive = false;
        snapshot_.bleConnected = false;
        snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "Advertising stopped");
    }
}

void FactoryApp::setBleConnectionState(bool connected)
{
    if (!snapshot_.bleActive) return;
    snapshot_.bleConnected = connected;
    if (connected) {
        snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "Connected");
    } else {
        BLEDevice::startAdvertising();
        snprintf(snapshot_.bleStatus, sizeof(snapshot_.bleStatus), "Disconnected");
    }
}

void FactoryApp::updateWifi()
{
    if (!wifiStarted_) return;
    if (wifiPingHandle_ && wifiPingDone_) {
        esp_ping_delete_session(wifiPingHandle_);
        wifiPingHandle_ = nullptr;
        wifiPingPending_ = false;
        snapshot_.wifiPingRunning = false;
        snapshot_.wifiPingOk = wifiPingReplies_ > 0;
        if (snapshot_.wifiPingOk) {
            snprintf(snapshot_.wifiPingStatus, sizeof(snapshot_.wifiPingStatus),
                     "baidu.com reachable  %ums", (unsigned)wifiPingLastMs_);
        } else {
            snprintf(snapshot_.wifiPingStatus, sizeof(snapshot_.wifiPingStatus), "baidu.com unreachable");
        }
    }
    if (wifiScanPending_) {
        const int16_t scanResult = WiFi.scanComplete();
        if (scanResult >= 0) {
            snapshot_.wifiNetworkCount = (uint8_t)min<int16_t>(scanResult, FACTORY_MAX_WIFI_NETWORKS);
            for (uint8_t i = 0; i < snapshot_.wifiNetworkCount; ++i) {
                snprintf(snapshot_.wifiNetworks[i].ssid, sizeof(snapshot_.wifiNetworks[i].ssid), "%s", WiFi.SSID(i).c_str());
                snapshot_.wifiNetworks[i].rssi = WiFi.RSSI(i);
                snapshot_.wifiNetworks[i].channel = WiFi.channel(i);
                snapshot_.wifiNetworks[i].encrypted = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            }
            WiFi.scanDelete();
            wifiScanPending_ = false;
            snapshot_.wifiScanning = false;
            snapshot_.wifiScanGeneration++;
            snprintf(snapshot_.wifiStatus, sizeof(snapshot_.wifiStatus), "%u networks found", snapshot_.wifiNetworkCount);
        }
    }
    const wl_status_t status = WiFi.status();
    snapshot_.wifiConnected = status == WL_CONNECTED;
    if (snapshot_.wifiConnecting) {
        if (status == WL_CONNECTED) {
            snapshot_.wifiConnecting = false;
            String ip = WiFi.localIP().toString();
            snprintf(snapshot_.wifiIp, sizeof(snapshot_.wifiIp), "%s", ip.c_str());
            snprintf(snapshot_.wifiStatus, sizeof(snapshot_.wifiStatus), "Connected to %s", WiFi.SSID().c_str());
            startNetworkTimeSync();
        } else if (millis() - wifiConnectStartedMs_ > 15000) {
            snapshot_.wifiConnecting = false;
            snprintf(snapshot_.wifiStatus, sizeof(snapshot_.wifiStatus), "Connection timed out");
        }
    }
    if (snapshot_.wifiConnected && snapshot_.wifiIp[0] == '\0') {
        String ip = WiFi.localIP().toString();
        snprintf(snapshot_.wifiIp, sizeof(snapshot_.wifiIp), "%s", ip.c_str());
    }
    if (snapshot_.wifiConnected) startNetworkTimeSync();
}

void FactoryApp::startNetworkTimeSync()
{
    if (networkTimeSyncStarted_) return;
    configTzTime(FACTORY_TIMEZONE, FACTORY_NTP_SERVER1, FACTORY_NTP_SERVER2);
    networkTimeSyncStarted_ = true;
    networkTimeSynced_ = false;
}

void FactoryApp::updateClock()
{
    const uint32_t nowMs = millis();
    if (lastClockUpdateMs_ && nowMs - lastClockUpdateMs_ < 250) return;
    lastClockUpdateMs_ = nowMs;

    if (networkTimeSyncStarted_ && !networkTimeSynced_ &&
        sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
        networkTimeSynced_ = true;
        if (snapshot_.hasRtc) {
            amoled_.rtc()->hwClockWrite();
        }
    }

    if (snapshot_.hasRtc) {
        RTC_DateTime rtc = amoled_.rtc()->getDateTime();
        snprintf(snapshot_.statusTime, sizeof(snapshot_.statusTime), "%02u:%02u:%02u",
                 rtc.getHour(), rtc.getMinute(), rtc.getSecond());
        snprintf(snapshot_.clock, sizeof(snapshot_.clock), "%04u-%02u-%02u  %02u:%02u:%02u",
                 rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHour(), rtc.getMinute(), rtc.getSecond());
        snprintf(snapshot_.rtc, sizeof(snapshot_.rtc), "RTC: %04u-%02u-%02u %02u:%02u:%02u",
                 rtc.getYear(), rtc.getMonth(), rtc.getDay(), rtc.getHour(), rtc.getMinute(), rtc.getSecond());
        return;
    }

    struct tm now = {};
    if (getLocalTime(&now, 10)) {
        snprintf(snapshot_.statusTime, sizeof(snapshot_.statusTime), "%02d:%02d:%02d",
                 now.tm_hour, now.tm_min, now.tm_sec);
        snprintf(snapshot_.clock, sizeof(snapshot_.clock), "%04d-%02d-%02d  %02d:%02d:%02d",
                 now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
        snprintf(snapshot_.rtc, sizeof(snapshot_.rtc), "ESP RTC: %04d-%02d-%02d %02d:%02d:%02d",
                 now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
    } else {
        snprintf(snapshot_.statusTime, sizeof(snapshot_.statusTime), "--:--:--");
        snprintf(snapshot_.clock, sizeof(snapshot_.clock), "Time unavailable");
        snprintf(snapshot_.rtc, sizeof(snapshot_.rtc), "ESP RTC: unavailable");
    }
}

void FactoryApp::runSdTestStep()
{
    if (!sdTestActive_) return;
    const uint32_t now = millis();
    if (sdTestPhase_ == 0) {
        sdTestStartMs_ = now;
        if (!amoled_.installSD()) {
            sdTestActive_ = false;
            snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "SD card not detected");
            return;
        }
        snapshot_.sdMountMs = millis() - sdTestStartMs_;
        snapshot_.sdCapacityMb = (uint32_t)(SD.cardSize() / (1024ULL * 1024ULL));
        memset(sdTestBuffer_, 0xA5, sizeof(sdTestBuffer_));
        sdTestFile_ = SD.open("/factoryv2-speed.bin", FILE_WRITE);
        if (!sdTestFile_) {
            sdTestActive_ = false;
            snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "SD write open failed");
            return;
        }
        sdTestBytes_ = 0;
        sdWriteStartMs_ = now;
        sdTestPhase_ = 1;
        snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "Testing write speed...");
        return;
    }
    if (sdTestPhase_ == 1) {
        const size_t chunk = min((uint32_t)sizeof(sdTestBuffer_), sdTestTargetBytes_ - sdTestBytes_);
        if (chunk) {
            if (sdTestFile_.write(sdTestBuffer_, chunk) == chunk) sdTestBytes_ += chunk;
            else {
                sdTestFile_.close();
                sdTestActive_ = false;
                snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "SD write failed");
                return;
            }
        }
        snapshot_.sdTestProgress = 10 + (sdTestBytes_ * 40 / sdTestTargetBytes_);
        if (sdTestBytes_ >= sdTestTargetBytes_) {
            sdTestFile_.flush();
            sdTestFile_.close();
            const uint32_t elapsed = max(1UL, now - sdWriteStartMs_);
            snapshot_.sdWriteKbps = (sdTestTargetBytes_ / 1024UL) * 1000UL / elapsed;
            sdTestFile_ = SD.open("/factoryv2-speed.bin", FILE_READ);
            sdTestBytes_ = 0;
            sdReadStartMs_ = now;
            sdTestPhase_ = 2;
            snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "Testing read speed...");
        }
        return;
    }
    if (sdTestPhase_ == 2) {
        const size_t chunk = min((uint32_t)sizeof(sdTestBuffer_), sdTestTargetBytes_ - sdTestBytes_);
        if (chunk) {
            if (sdTestFile_.read(sdTestBuffer_, chunk) == chunk) sdTestBytes_ += chunk;
            else {
                sdTestFile_.close();
                SD.remove("/factoryv2-speed.bin");
                sdTestActive_ = false;
                snprintf(snapshot_.sdTestStatus, sizeof(snapshot_.sdTestStatus), "SD read failed");
                return;
            }
        }
        snapshot_.sdTestProgress = 50 + (sdTestBytes_ * 40 / sdTestTargetBytes_);
        if (sdTestBytes_ >= sdTestTargetBytes_) {
            sdTestFile_.close();
            SD.remove("/factoryv2-speed.bin");
            const uint32_t elapsed = max(1UL, now - sdReadStartMs_);
            snapshot_.sdReadKbps = (sdTestTargetBytes_ / 1024UL) * 1000UL / elapsed;
            snapshot_.sdTestProgress = 100;
            snapshot_.sdTestStatus[0] = '\0';
            sdTestActive_ = false;
            sdTestPhase_ = 3;
        }
    }
}

void FactoryApp::startSdMusicScan()
{
    if (sdMusicDirectory_) sdMusicDirectory_.close();
    shield_.stopActiveTests();
    sdMusicPlaybackActive_ = false;
    sdMusicScanActive_ = snapshot_.hasSd && snapshot_.shieldAudioReady;
    sdMusicScanPhase_ = 0;
    snapshot_.sdMusicTrackCount = 0;
    snapshot_.sdMusicCurrentPath[0] = '\0';
    memset(snapshot_.sdMusicTracks, 0, sizeof(snapshot_.sdMusicTracks));
    snapshot_.sdMusicScanning = sdMusicScanActive_;
    snapshot_.sdMusicGeneration++;
    snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "%s",
             sdMusicScanActive_ ? "Preparing SD card..." : "SD audio unavailable");
}

void FactoryApp::finishSdMusicScan(const char *status)
{
    if (sdMusicDirectory_) sdMusicDirectory_.close();
    sdMusicScanActive_ = false;
    snapshot_.sdMusicScanning = false;
    snapshot_.sdMusicGeneration++;
    snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "%s", status ? status : "Scan complete");
}

void FactoryApp::runSdMusicScanStep()
{
    if (!sdMusicScanActive_) return;
    if (sdTestActive_) {
        snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Waiting for SD test...");
        return;
    }
    if (snapshot_.sdMusicTrackCount >= FACTORY_MAX_SD_MUSIC_TRACKS) {
        char status[64];
        snprintf(status, sizeof(status), "Showing first %u MP3 files", FACTORY_MAX_SD_MUSIC_TRACKS);
        finishSdMusicScan(status);
        return;
    }
    if (sdMusicScanPhase_ == 0) {
        if (!amoled_.installSD()) {
            finishSdMusicScan("SD card not detected");
            return;
        }
        snapshot_.sdCapacityMb = (uint32_t)(SD.cardSize() / (1024ULL * 1024ULL));
        sdMusicDirectory_ = SD.open("/");
        if (!sdMusicDirectory_ || !sdMusicDirectory_.isDirectory()) {
            finishSdMusicScan("Unable to open SD root");
            return;
        }
        sdMusicScanPhase_ = 1;
        snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Scanning / ...");
        return;
    }

    File entry = sdMusicDirectory_.openNextFile();
    if (!entry) {
        sdMusicDirectory_.close();
        if (sdMusicScanPhase_ == 1) {
            sdMusicDirectory_ = SD.open("/music");
            if (sdMusicDirectory_ && sdMusicDirectory_.isDirectory()) {
                sdMusicScanPhase_ = 2;
                snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Scanning /music ...");
                return;
            }
            if (sdMusicDirectory_) sdMusicDirectory_.close();
        }
        char status[64];
        if (snapshot_.sdMusicTrackCount) {
            snprintf(status, sizeof(status), "%u MP3 file%s found", snapshot_.sdMusicTrackCount,
                     snapshot_.sdMusicTrackCount == 1 ? "" : "s");
        } else {
            snprintf(status, sizeof(status), "No MP3 files in / or /music");
        }
        finishSdMusicScan(status);
        return;
    }
    if (!entry.isDirectory() && isMp3Path(entry.path()) &&
        strlen(entry.path()) < sizeof(snapshot_.sdMusicTracks[0].path)) {
        FactorySdMusicTrack &track = snapshot_.sdMusicTracks[snapshot_.sdMusicTrackCount++];
        snprintf(track.name, sizeof(track.name), "%s", entry.name());
        snprintf(track.path, sizeof(track.path), "%s", entry.path());
        track.sizeBytes = (uint32_t)entry.size();
        snprintf(snapshot_.sdMusicStatus, sizeof(snapshot_.sdMusicStatus), "Scanning... %u MP3 found",
                 snapshot_.sdMusicTrackCount);
    }
    entry.close();
}

void FactoryApp::pixelTestAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app->pixels_) return;
    app->pixels_->setPixelColor(0, app->pixels_->Color(255, 80, 0));
    app->pixels_->show();
    app->ui_.showTestMessage("RGB test: orange");
}

void FactoryApp::shieldTestAction(void *context, uint8_t testId)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app) return;
    static const char *const testNames[] = {
        "Audio playback", "Microphone", "LoRa TX", "LoRa RX",
        "IR TX", "IR RX", "Sensors"
    };
    const bool ready = app->shield_.runTest(testId);
    const char *name = testId == 17 ? "MP3 playback" :
                       (testId == 16 ? "Beep playback" :
                       ((testId >= 1 && testId <= 7) ? testNames[testId - 1] : "Shield test"));
    if (ready) {
        app->ui_.showTestMessage(app->shield_.lastStatus());
    } else {
        char message[64];
        snprintf(message, sizeof(message), "%s: unavailable", name);
        app->ui_.showTestMessage(message);
    }
}

void FactoryApp::powerOffAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || !app->snapshot_.hasPmic) return;
    if (app->amoled_.isVbusIn() || app->amoled_.getVbusVoltage() > 4500) {
        app->ui_.showPowerOffDialog(true);
        return;
    }
    app->ui_.showPowerOffDialog(false);
}

void FactoryApp::powerOffConfirmedAction(void *context)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || !app->snapshot_.hasPmic) return;
    // Re-check the cable after the user has confirmed. USB may have been
    // connected while the warning dialog was open.
    if (app->amoled_.isVbusIn() || app->amoled_.getVbusVoltage() > 4500) {
        app->ui_.showPowerOffDialog(true);
        return;
    }
    PmicBase *pmic = app->amoled_.pmic();
    if (!pmic || !pmic->getPower().enableShipMode(true)) {
        app->ui_.showTestMessage("Shipping mode is not supported");
        return;
    }
    app->ui_.showTestMessage("Entering shipping mode...");
    delay(150);
    esp_deep_sleep_start();
}

void FactoryApp::extensionAction(void *context, uint8_t slot, uint8_t action)
{
    FactoryApp *app = static_cast<FactoryApp *>(context);
    if (!app || slot >= app->extensionCount_ || !app->extensions_[slot]) return;
    app->extensions_[slot]->action(action);
    app->ui_.showTestMessage("Extension action sent");
    app->updateSnapshot();
}
