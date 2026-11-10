#pragma once

#include <lvgl.h>
#include "FactoryData.h"

enum FactoryFeature : uint8_t {
    FACTORY_FEATURE_DISPLAY = 0,
    FACTORY_FEATURE_SD,
    FACTORY_FEATURE_RGB,
    FACTORY_FEATURE_TOUCH,
    FACTORY_FEATURE_POWER,
    FACTORY_FEATURE_POWEROFF,
    FACTORY_FEATURE_AUDIO,
    FACTORY_FEATURE_SD_MUSIC,
    FACTORY_FEATURE_MIC,
    FACTORY_FEATURE_LORA_TX,
    FACTORY_FEATURE_LORA_RX,
    FACTORY_FEATURE_LORA_SETTINGS,
    FACTORY_FEATURE_IR_TX,
    FACTORY_FEATURE_IR_RX,
    FACTORY_FEATURE_IMU,
    FACTORY_FEATURE_MAGNETOMETER,
    FACTORY_FEATURE_LIVE,
    FACTORY_FEATURE_INFO,
    FACTORY_FEATURE_WIFI,
    FACTORY_FEATURE_BLE,
    FACTORY_FEATURE_EXTENSIONS,
    FACTORY_FEATURE_COUNT
};

class FactoryUi {
public:
    void begin(const FactorySnapshot &snapshot, const FactoryActions &actions);
    void rebuild(const FactorySnapshot &snapshot);
    void update(const FactorySnapshot &snapshot);
    void goHome();
    void showTestMessage(const char *message);
    void showPowerOffDialog(bool usbConnected);
    void showColorTest(lv_color_t color);

private:
    static void actionEvent(lv_event_t *event);
    static void navigateEvent(lv_event_t *event);
    static void brightnessEvent(lv_event_t *event);
    static void audioVolumeEvent(lv_event_t *event);
    static void micSensitivityEvent(lv_event_t *event);
    static void pulseScaleEvent(void *object, int32_t value);
    static void tileChangedEvent(lv_event_t *event);
    static void colorTestCloseEvent(lv_event_t *event);
    static void powerOffDialogEvent(lv_event_t *event);

    lv_obj_t *makeLabel(lv_obj_t *parent, const char *text, uint32_t size = 18);
    lv_obj_t *makeIconImage(lv_obj_t *parent, const lv_img_dsc_t *icon, uint16_t size = 40);
    lv_obj_t *createPage(lv_obj_t *tileview, const lv_img_dsc_t *icon, const char *title,
                         uint8_t index, uint16_t width, uint16_t height);
    lv_obj_t *createFeatureTile(lv_obj_t *parent, const lv_img_dsc_t *icon, const char *title,
                                uint8_t page, lv_color_t color, uint16_t width, uint16_t height);
    lv_obj_t *createActionTile(lv_obj_t *parent, const lv_img_dsc_t *icon, const char *title,
                               uint32_t action, lv_color_t color, uint16_t width, uint16_t height);
    lv_obj_t *createActionButton(lv_obj_t *parent, const char *title, uint32_t action,
                                 lv_color_t color, uint16_t width, uint16_t height);
    lv_obj_t *createMetric(lv_obj_t *parent, uint16_t width, uint16_t height, const char *title,
                           const char *value, lv_obj_t **valueLabel);
    lv_obj_t *createSettingSlider(lv_obj_t *parent, const char *title, int32_t minValue,
                                  int32_t maxValue, int32_t value, uint32_t action);
    void createFeaturePage(const FactorySnapshot &snapshot, FactoryFeature feature,
                           uint8_t page, const lv_img_dsc_t *icon, const char *title,
                           const char *description, lv_color_t color);
    void createUi(const FactorySnapshot &snapshot);
    void activatePage(uint8_t page);
    void addFeature(const FactorySnapshot &snapshot, FactoryFeature feature,
                    const lv_img_dsc_t *icon, const char *title, const char *description,
                    lv_color_t color);
    bool featureAvailable(const FactorySnapshot &snapshot, FactoryFeature feature) const;

    FactoryActions actions_;
    lv_obj_t *tileview_ = nullptr;
    lv_obj_t *activePageRoot_ = nullptr;
    lv_obj_t *selector_ = nullptr;
    lv_obj_t *colorTestOverlay_ = nullptr;
    lv_obj_t *powerOffDialog_ = nullptr;
    lv_obj_t *timeLabel_ = nullptr;
    lv_obj_t *uptimeLabel_ = nullptr;
    lv_obj_t *touchLabel_ = nullptr;
    lv_obj_t *powerLabel_ = nullptr;
    lv_obj_t *sensorLabel_ = nullptr;
    lv_obj_t *rtcLabel_ = nullptr;
    lv_obj_t *sdLabel_ = nullptr;
    lv_obj_t *memoryLabel_ = nullptr;
    lv_obj_t *sdTestLabel_ = nullptr;
    lv_obj_t *powerBatteryValue_ = nullptr;
    lv_obj_t *powerVbusValue_ = nullptr;
    lv_obj_t *powerChargeValue_ = nullptr;
    lv_obj_t *powerPmicValue_ = nullptr;
    lv_obj_t *powerBar_ = nullptr;
    lv_obj_t *touchCoordinateLabel_ = nullptr;
    lv_obj_t *audioVolumeSlider_ = nullptr;
    lv_obj_t *sdMusicStatusLabel_ = nullptr;
    lv_obj_t *sdMusicList_ = nullptr;
    lv_obj_t *sdMusicButtons_[FACTORY_MAX_SD_MUSIC_TRACKS] = {};
    lv_obj_t *micLevelBar_ = nullptr;
    lv_obj_t *micLevelLabel_ = nullptr;
    lv_obj_t *micSensitivitySlider_ = nullptr;
    lv_obj_t *micSensitivityLabel_ = nullptr;
    int32_t micSensitivity_ = 5;
    lv_obj_t *loraTxParamsLabel_ = nullptr;
    lv_obj_t *loraRxParamsLabel_ = nullptr;
    lv_obj_t *loraTxMessageLabel_ = nullptr;
    lv_obj_t *loraTxCountdownLabel_ = nullptr;
    lv_obj_t *loraTxButton_ = nullptr;
    lv_obj_t *loraRxMessageLabel_ = nullptr;
    lv_obj_t *loraTxStateLabel_ = nullptr;
    lv_obj_t *loraTxCountLabel_ = nullptr;
    lv_obj_t *loraRxStateLabel_ = nullptr;
    lv_obj_t *loraRxCountLabel_ = nullptr;
    lv_obj_t *loraSettingsLabel_ = nullptr;
    lv_obj_t *irTxDetailLabel_ = nullptr;
    lv_obj_t *irRxDetailLabel_ = nullptr;
    lv_obj_t *irTxIndicator_ = nullptr;
    lv_obj_t *irTxStateLabel_ = nullptr;
    lv_obj_t *irTxCountLabel_ = nullptr;
    lv_obj_t *irTxButton_ = nullptr;
    lv_obj_t *irRxProtocolLabel_ = nullptr;
    lv_obj_t *irRxRawLabel_ = nullptr;
    lv_obj_t *imuChart_ = nullptr;
    lv_chart_series_t *imuSeriesX_ = nullptr;
    lv_chart_series_t *imuSeriesY_ = nullptr;
    lv_chart_series_t *imuSeriesZ_ = nullptr;
    lv_obj_t *imuLevelLed_ = nullptr;
    lv_obj_t *imuLevelLabel_ = nullptr;
    lv_obj_t *imuValueLabel_ = nullptr;
    lv_obj_t *gyroValueLabel_ = nullptr;
    lv_obj_t *compassNeedle_ = nullptr;
    lv_point_precise_t compassNeedlePoints_[2] = {};
    lv_obj_t *compassHeadingLabel_ = nullptr;
    lv_obj_t *wifiStatusLabel_ = nullptr;
    lv_obj_t *wifiInfoLabel_ = nullptr;
    lv_obj_t *wifiPingLabel_ = nullptr;
    lv_obj_t *wifiPingButton_ = nullptr;
    lv_obj_t *wifiNetworkButtons_[FACTORY_MAX_WIFI_NETWORKS] = {};
    lv_obj_t *wifiNetworkLabels_[FACTORY_MAX_WIFI_NETWORKS] = {};
    lv_obj_t *wifiNetworkRssiLabels_[FACTORY_MAX_WIFI_NETWORKS] = {};
    lv_obj_t *wifiNetworkSignalBars_[FACTORY_MAX_WIFI_NETWORKS][4] = {};
    lv_obj_t *wifiNetworkIcons_[FACTORY_MAX_WIFI_NETWORKS] = {};
    lv_obj_t *bleStatusLabel_ = nullptr;
    lv_obj_t *infoStatusLabels_[14] = {};
    uint32_t lastWifiScanGeneration_ = 0;
    lv_obj_t *magValueLabel_ = nullptr;
    lv_obj_t *magCalibrationLabel_ = nullptr;
    lv_obj_t *magCalibrationBar_ = nullptr;
    lv_obj_t *magCalibrationButton_ = nullptr;
    lv_obj_t *touchCrosshairH_ = nullptr;
    lv_obj_t *touchCrosshairV_ = nullptr;
    lv_obj_t *micChart_ = nullptr;
    lv_chart_series_t *micSeries_ = nullptr;
    lv_obj_t *featureMessage_[FACTORY_FEATURE_COUNT] = {};
    lv_obj_t *extensionValueLabels_[FACTORY_MAX_EXTENSIONS] = {};
    uint16_t launcherTileWidth_ = 104;
    uint16_t launcherTileHeight_ = 104;
    uint8_t featureCount_ = 0;
    FactoryFeature pageFeatures_[FACTORY_FEATURE_COUNT + 1] = {};
    lv_obj_t *pageStatusLed_[FACTORY_FEATURE_COUNT + 1] = {};
    lv_obj_t *pageTimeLabels_[FACTORY_FEATURE_COUNT + 1] = {};
    char lastStatusTime_[9] = {};
    uint8_t activePage_ = 0;
    uint32_t lastLoraTxCount_ = 0;
    uint32_t lastIrTxCount_ = 0;
    uint32_t lastImuChartMs_ = 0;
    uint32_t lastSdMusicGeneration_ = UINT32_MAX;
};
