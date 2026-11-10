#include "FactoryUi.h"

#include <Arduino.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "icons/FactoryIcons.h"
#include "icons/FactoryIconsExtra.h"

namespace {
const lv_color_t kBackground = lv_color_make(8, 13, 22);
const lv_color_t kPanel = lv_color_make(18, 28, 42);
const lv_color_t kPanelPressed = lv_color_make(29, 48, 64);
const lv_color_t kBorder = lv_color_make(45, 65, 86);
const lv_color_t kText = lv_color_make(235, 243, 250);
const lv_color_t kMuted = lv_color_make(145, 165, 184);
const lv_color_t kCyan = lv_color_make(78, 218, 205);
const lv_color_t kGreen = lv_color_make(103, 226, 139);
const lv_color_t kAmber = lv_color_make(255, 188, 91);
const lv_color_t kRed = lv_color_make(255, 111, 111);

constexpr int32_t kLiveLandscapeColumns[] = {
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};
constexpr int32_t kLiveLandscapeRows[] = {
    42, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};
constexpr int32_t kLivePortraitColumns[] = {
    LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};
constexpr int32_t kLivePortraitRows[] = {
    52, 62, 62, 62, 62, LV_GRID_TEMPLATE_LAST,
};

constexpr uint8_t kHomePage = 0;
constexpr uint32_t kActionDisplay = 1;
constexpr uint32_t kActionSd = 2;
constexpr uint32_t kActionRgb = 3;
constexpr uint32_t kActionRotation = 4;
constexpr uint32_t kActionPowerOff = 5;
constexpr uint32_t kActionMagCalibrate = 6;
constexpr uint32_t kActionShieldBase = 10;
constexpr uint32_t kActionLoraConfigBase = 100;
constexpr uint32_t kActionWifiNetworkBase = 200;
constexpr uint32_t kActionWifiPing = 220;
constexpr uint32_t kActionSdMusicTrackBase = 300;

void makeStatic(lv_obj_t *object)
{
    lv_obj_set_scrollable(object, false);
    lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_OFF);
}

void stylePanel(lv_obj_t *object, lv_color_t color = kPanel)
{
    lv_obj_set_style_bg_color(object, color, 0);
    lv_obj_set_style_border_width(object, 1, 0);
    lv_obj_set_style_border_color(object, kBorder, 0);
    lv_obj_set_style_radius(object, 8, 0);
}

lv_obj_t *createStatusLed(lv_obj_t *parent)
{
    lv_obj_t *led = lv_obj_create(parent);
    makeStatic(led);
    lv_obj_set_size(led, 11, 11);
    lv_obj_set_style_radius(led, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(led, 1, 0);
    lv_obj_set_style_border_color(led, kText, 0);
    lv_obj_set_style_bg_color(led, kRed, 0);
    return led;
}

void setStatusLed(lv_obj_t *led, bool ready, bool fault)
{
    if (!led) return;
    const lv_color_t color = ready && !fault ? kGreen : kRed;
    lv_obj_set_style_bg_color(led, color, 0);
    lv_obj_set_style_border_color(led, color, 0);
}
}

lv_obj_t *FactoryUi::makeLabel(lv_obj_t *parent, const char *text, uint32_t size)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text ? text : "");
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    makeStatic(label);
    lv_obj_set_style_text_font(label, size <= 18 ? &lv_font_montserrat_18 : &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, kText, 0);
    return label;
}

lv_obj_t *FactoryUi::makeIconImage(lv_obj_t *parent, const lv_img_dsc_t *icon, uint16_t size)
{
    lv_obj_t *image = lv_image_create(parent);
    lv_image_set_src(image, icon);
    makeStatic(image);
    lv_obj_set_size(image, size, size);
    // Keep the generated RGB565A8 pixels on the native rendering path. The
    // software transform path can corrupt these small title-bar images on
    // the target display; centering avoids that while preserving the full
    // 40px launcher/hero artwork.
    lv_image_set_inner_align(image, LV_IMAGE_ALIGN_CENTER);
    lv_obj_set_style_image_opa(image, LV_OPA_COVER, 0);
    return image;
}

lv_obj_t *FactoryUi::createPage(lv_obj_t *tileview, const lv_img_dsc_t *icon, const char *title,
                                uint8_t index, uint16_t width, uint16_t height)
{
    // Function pages are entered from an app tile. Only the icon strip scrolls;
    // the hardware Home button is the consistent way back to the app launcher.
    lv_obj_t *page = lv_tileview_add_tile(tileview, index, 0, LV_DIR_NONE);
    activePageRoot_ = page;
    lv_obj_set_user_data(page, (void *)(uintptr_t)index);
    makeStatic(page);
    lv_obj_set_style_bg_color(page, kBackground, 0);
    lv_obj_set_style_pad_all(page, 7, 0);
    lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(page, 5, 0);
    if (index != kHomePage) {
        lv_obj_set_scrollable(page, true);
        lv_obj_set_scroll_dir(page, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    }

    lv_obj_t *header = lv_obj_create(page);
    makeStatic(header);
    lv_obj_set_width(header, lv_pct(100));
    lv_obj_set_height(header, 26);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_border_color(header, kBorder, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_pad_all(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    makeIconImage(header, icon, 24);
    lv_obj_t *heading = makeLabel(header, title, 18);
    lv_obj_set_flex_grow(heading, 1);
    lv_obj_set_style_text_color(heading, kText, 0);
    if (width >= height) {
        pageTimeLabels_[index] = makeLabel(header, "--:--:--", 16);
        lv_obj_set_floating(pageTimeLabels_[index], true);
        lv_obj_set_width(pageTimeLabels_[index], 76);
        lv_obj_set_style_text_align(pageTimeLabels_[index], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(pageTimeLabels_[index], LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(pageTimeLabels_[index], kMuted, 0);
        if (index != kHomePage) pageStatusLed_[index] = createStatusLed(header);
    }

    lv_obj_t *body = lv_obj_create(page);
    makeStatic(body);
    lv_obj_set_width(body, lv_pct(100));
    lv_obj_set_height(body, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(body, index == kHomePage ? 1 : 0);
    lv_obj_set_style_bg_opa(body, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(body, 0, 0);
    lv_obj_set_style_pad_all(body, 0, 0);
    lv_obj_set_style_pad_row(body, 5, 0);
    lv_obj_set_style_pad_column(body, 5, 0);
    lv_obj_set_flex_flow(body, LV_FLEX_FLOW_COLUMN);
    (void)width;
    (void)height;
    return body;
}

lv_obj_t *FactoryUi::createFeatureTile(lv_obj_t *parent, const lv_img_dsc_t *icon, const char *title,
                                       uint8_t page, lv_color_t color, uint16_t width, uint16_t height)
{
    lv_obj_t *tile = lv_button_create(parent);
    makeStatic(tile);
    lv_obj_set_size(tile, width, height);
    stylePanel(tile);
    lv_obj_set_style_bg_color(tile, kPanelPressed, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(tile, color, LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(tile, 6, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    makeIconImage(tile, icon, 40);
    lv_obj_t *label = makeLabel(tile, title, 16);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_user_data(tile, (void *)(uintptr_t)page);
    lv_obj_add_event_cb(tile, navigateEvent, LV_EVENT_CLICKED, this);
    return tile;
}

lv_obj_t *FactoryUi::createActionTile(lv_obj_t *parent, const lv_img_dsc_t *icon, const char *title,
                                      uint32_t action, lv_color_t color, uint16_t width, uint16_t height)
{
    lv_obj_t *tile = lv_button_create(parent);
    makeStatic(tile);
    lv_obj_set_size(tile, width, height);
    stylePanel(tile);
    lv_obj_set_style_bg_color(tile, kPanelPressed, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(tile, color, 0);
    lv_obj_set_style_pad_all(tile, 6, 0);
    lv_obj_set_flex_flow(tile, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(tile, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    makeIconImage(tile, icon, 40);
    lv_obj_t *label = makeLabel(tile, title, 16);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_user_data(tile, (void *)(uintptr_t)action);
    lv_obj_add_event_cb(tile, actionEvent, LV_EVENT_CLICKED, this);
    return tile;
}

lv_obj_t *FactoryUi::createActionButton(lv_obj_t *parent, const char *title, uint32_t action,
                                        lv_color_t color, uint16_t width, uint16_t height)
{
    lv_obj_t *button = lv_button_create(parent);
    makeStatic(button);
    lv_obj_set_size(button, width, height);
    lv_obj_set_style_transform_pivot_x(button, width / 2, 0);
    lv_obj_set_style_transform_pivot_y(button, height / 2, 0);
    lv_obj_set_style_bg_color(button, color, 0);
    lv_obj_set_style_bg_color(button, kPanelPressed, LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 7, 0);
    lv_obj_set_user_data(button, (void *)(uintptr_t)action);
    lv_obj_add_event_cb(button, actionEvent, LV_EVENT_CLICKED, this);
    lv_obj_t *label = makeLabel(button, title, 18);
    lv_obj_set_style_text_color(label, kBackground, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
    return button;
}

lv_obj_t *FactoryUi::createMetric(lv_obj_t *parent, uint16_t width, uint16_t height,
                                  const char *title, const char *value, lv_obj_t **valueLabel)
{
    lv_obj_t *panel = lv_obj_create(parent);
    makeStatic(panel);
    lv_obj_set_size(panel, width, height);
    stylePanel(panel);
    lv_obj_set_style_pad_left(panel, 8, 0);
    lv_obj_set_style_pad_right(panel, 8, 0);
    lv_obj_set_style_pad_top(panel, 3, 0);
    lv_obj_set_style_pad_bottom(panel, 3, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_t *titleLabel = makeLabel(panel, title, 16);
    lv_obj_set_style_text_color(titleLabel, kMuted, 0);
    *valueLabel = makeLabel(panel, value, 16);
    lv_obj_set_width(*valueLabel, lv_pct(100));
    return panel;
}

lv_obj_t *FactoryUi::createSettingSlider(lv_obj_t *parent, const char *title, int32_t minValue,
                                         int32_t maxValue, int32_t value, uint32_t action)
{
    lv_obj_t *label = makeLabel(parent, title, 16);
    lv_obj_set_width(label, lv_pct(100));
    lv_obj_t *slider = lv_slider_create(parent);
    makeStatic(slider);
    lv_obj_set_width(slider, lv_pct(100));
    lv_slider_set_range(slider, minValue, maxValue);
    lv_slider_set_value(slider, value, LV_ANIM_OFF);
    lv_obj_set_user_data(slider, (void *)(uintptr_t)action);
    lv_obj_add_event_cb(slider, actionEvent, LV_EVENT_VALUE_CHANGED, this);
    return slider;
}

bool FactoryUi::featureAvailable(const FactorySnapshot &snapshot, FactoryFeature feature) const
{
    switch (feature) {
    case FACTORY_FEATURE_DISPLAY: return true;
    case FACTORY_FEATURE_SD: return snapshot.hasSd;
    case FACTORY_FEATURE_RGB: return snapshot.hasPixels;
    case FACTORY_FEATURE_TOUCH: return snapshot.hasTouch;
    case FACTORY_FEATURE_POWER: return snapshot.hasPmic;
    case FACTORY_FEATURE_POWEROFF: return snapshot.hasPmic;
    case FACTORY_FEATURE_AUDIO:
        return snapshot.hasShield && snapshot.shieldAudioReady;
    case FACTORY_FEATURE_SD_MUSIC:
        return snapshot.hasSd && snapshot.hasShield && snapshot.shieldAudioReady;
    case FACTORY_FEATURE_MIC:
        return snapshot.hasShield && snapshot.shieldMicReady;
    case FACTORY_FEATURE_LORA_TX:
    case FACTORY_FEATURE_LORA_RX: return snapshot.hasShield && snapshot.shieldLoraReady;
    case FACTORY_FEATURE_LORA_SETTINGS: return snapshot.hasShield && snapshot.shieldLoraReady;
    case FACTORY_FEATURE_IR_TX:
    case FACTORY_FEATURE_IR_RX: return snapshot.hasShield && snapshot.shieldIrReady;
    case FACTORY_FEATURE_IMU: return snapshot.hasShield && snapshot.shieldImuReady;
    case FACTORY_FEATURE_MAGNETOMETER: return snapshot.hasShield && snapshot.shieldMagReady;
    case FACTORY_FEATURE_LIVE: return true;
    case FACTORY_FEATURE_INFO: return true;
    case FACTORY_FEATURE_WIFI: return snapshot.hasWifi;
    case FACTORY_FEATURE_BLE: return snapshot.hasBle;
    case FACTORY_FEATURE_EXTENSIONS: return snapshot.extensionCount > 0;
    default: return false;
    }
}

void FactoryUi::createFeaturePage(const FactorySnapshot &snapshot, FactoryFeature feature,
                                  uint8_t page, const lv_img_dsc_t *icon, const char *title,
                                  const char *description, lv_color_t color)
{
    lv_obj_t *body = createPage(tileview_, icon, title, page, snapshot.width, snapshot.height);
    const bool portrait = snapshot.height > snapshot.width;
    const uint16_t actionWidth = snapshot.width >= 500 ? 220 : snapshot.width - 14;
    if (feature == FACTORY_FEATURE_LIVE) {
        const uint16_t contentWidth = snapshot.width > 14 ? snapshot.width - 14 : snapshot.width;
        const uint16_t bodyHeight = snapshot.height > 45 ? snapshot.height - 45 : snapshot.height;
        lv_obj_set_height(body, portrait ? LV_SIZE_CONTENT : bodyHeight);
        lv_obj_set_flex_grow(body, 0);
        lv_obj_set_style_pad_all(body, 0, 0);
        lv_obj_set_style_pad_row(body, 6, 0);
        lv_obj_set_style_pad_column(body, 6, 0);
        lv_obj_set_layout(body, LV_LAYOUT_GRID);
        lv_obj_set_grid_dsc_array(body,
                                  portrait ? kLivePortraitColumns : kLiveLandscapeColumns,
                                  portrait ? kLivePortraitRows : kLiveLandscapeRows);

        auto placeMetric = [](lv_obj_t *metric, uint8_t column, uint8_t row) {
            // Let the grid cell, rather than the creation-time dimensions,
            // determine the final card size on both orientations.
            lv_obj_set_width(metric, LV_SIZE_CONTENT);
            lv_obj_set_height(metric, LV_SIZE_CONTENT);
            lv_obj_set_grid_cell(metric, LV_GRID_ALIGN_STRETCH, column, 1,
                                 LV_GRID_ALIGN_STRETCH, row, 1);
        };

        lv_obj_t *clockMetric = createMetric(body, contentWidth, 42,
                                             "Local time", snapshot.clock, &timeLabel_);
        placeMetric(clockMetric, 0, 0);
        lv_obj_set_grid_cell(clockMetric, LV_GRID_ALIGN_STRETCH, 0,
                             portrait ? 1 : 2, LV_GRID_ALIGN_STRETCH, 0, 1);

        lv_obj_t *uptimeMetric = createMetric(body, contentWidth, 62,
                                               "Uptime", "0s", &uptimeLabel_);
        lv_obj_t *touchMetric = createMetric(body, contentWidth, 62,
                                              "Touch", "IDLE", &touchLabel_);
        lv_obj_t *powerMetric = createMetric(body, contentWidth, 62,
                                              "Power", "N/A", &powerLabel_);
        lv_obj_t *memoryMetric = createMetric(body, contentWidth, 62,
                                               "Memory", "Heap: --", &memoryLabel_);
        if (portrait) {
            placeMetric(uptimeMetric, 0, 1);
            placeMetric(touchMetric, 0, 2);
            placeMetric(powerMetric, 0, 3);
            placeMetric(memoryMetric, 0, 4);
        } else {
            placeMetric(uptimeMetric, 0, 1);
            placeMetric(touchMetric, 1, 1);
            placeMetric(powerMetric, 0, 2);
            placeMetric(memoryMetric, 1, 2);
        }
        return;
    }

    if (feature == FACTORY_FEATURE_INFO) {
        const char *const names[] = {
            "DISPLAY", "TOUCH", "SD CARD", "POWER / PMIC", "HARDWARE RTC", "PSRAM", "SHIELD",
            "EXPANDER", "LORA", "CODEC", "MICROPHONE", "INFRARED", "IMU", "COMPASS"
        };
        const uint8_t rowCount = snapshot.hasShield ? 14 : 7;
        lv_obj_t *list = lv_obj_create(body);
        makeStatic(list);
        lv_obj_set_width(list, lv_pct(100));
        lv_obj_set_height(list, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(list, 0, 0);
        lv_obj_set_style_pad_all(list, 0, 0);
        lv_obj_set_style_pad_row(list, 6, 0);
        lv_obj_set_style_pad_column(list, 6, 0);
        lv_obj_set_flex_flow(list, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW_WRAP);
        const uint16_t rowWidth = portrait ? snapshot.width - 14 : (snapshot.width - 20) / 2;
        for (uint8_t i = 0; i < rowCount; ++i) {
            lv_obj_t *row = lv_obj_create(list);
            makeStatic(row);
            lv_obj_set_size(row, rowWidth, 36);
            stylePanel(row);
            lv_obj_set_style_pad_left(row, 9, 0);
            lv_obj_set_style_pad_right(row, 9, 0);
            lv_obj_set_style_pad_top(row, 0, 0);
            lv_obj_set_style_pad_bottom(row, 0, 0);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_t *name = makeLabel(row, names[i], 16);
            lv_obj_set_flex_grow(name, 1);
            infoStatusLabels_[i] = makeLabel(row, "FAIL", 16);
            lv_obj_set_width(infoStatusLabels_[i], 46);
            lv_obj_set_style_text_align(infoStatusLabels_[i], LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_style_text_color(infoStatusLabels_[i], kRed, 0);
        }
        return;
    }

    if (feature == FACTORY_FEATURE_TOUCH) {
        // Keep the crosshair outside the flex/scroll body so it can cover the
        // complete display, including the page header, without being clipped.
        lv_obj_t *screen = lv_screen_active();
        touchCrosshairH_ = lv_obj_create(screen);
        touchCrosshairV_ = lv_obj_create(screen);
        makeStatic(touchCrosshairH_); makeStatic(touchCrosshairV_);
        lv_obj_set_floating(touchCrosshairH_, true);
        lv_obj_set_floating(touchCrosshairV_, true);
        lv_obj_set_style_bg_color(touchCrosshairH_, color, 0);
        lv_obj_set_style_bg_color(touchCrosshairV_, color, 0);
        lv_obj_set_style_border_width(touchCrosshairH_, 0, 0);
        lv_obj_set_style_border_width(touchCrosshairV_, 0, 0);
        lv_obj_set_size(touchCrosshairH_, lv_pct(100), 2);
        lv_obj_set_size(touchCrosshairV_, 2, lv_pct(100));
        lv_obj_set_pos(touchCrosshairH_, 0, 0);
        lv_obj_set_pos(touchCrosshairV_, 0, 0);
        lv_obj_set_hidden(touchCrosshairH_, true);
        lv_obj_set_hidden(touchCrosshairV_, true);
        touchCoordinateLabel_ = makeLabel(screen, "", snapshot.width >= 500 ? 32 :
                                          (snapshot.height > snapshot.width ? 20 : 28));
        lv_obj_set_width(touchCoordinateLabel_, lv_pct(100));
        lv_obj_set_align(touchCoordinateLabel_, LV_ALIGN_CENTER);
        lv_obj_set_style_text_align(touchCoordinateLabel_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_bg_opa(touchCoordinateLabel_, LV_OPA_TRANSP, 0);
        lv_obj_set_hidden(touchCoordinateLabel_, true);
        return;
    }

    if (feature == FACTORY_FEATURE_POWER) {
        const uint16_t metricWidth = portrait ? snapshot.width - 14 : (snapshot.width - 19) / 2;
        lv_obj_t *powerRow = lv_obj_create(body);
        makeStatic(powerRow);
        lv_obj_set_width(powerRow, lv_pct(100));
        lv_obj_set_height(powerRow, portrait ? 124 : 62);
        lv_obj_set_style_bg_opa(powerRow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(powerRow, 0, 0);
        lv_obj_set_style_pad_all(powerRow, 0, 0);
        lv_obj_set_flex_flow(powerRow, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
        createMetric(powerRow, metricWidth, portrait ? 58 : 62, "Battery", "-- mV", &powerBatteryValue_);
        createMetric(powerRow, metricWidth, portrait ? 58 : 62, "USB / VBUS", "-- mV", &powerVbusValue_);
        powerRow = lv_obj_create(body);
        makeStatic(powerRow);
        lv_obj_set_width(powerRow, lv_pct(100));
        lv_obj_set_height(powerRow, portrait ? 124 : 62);
        lv_obj_set_style_bg_opa(powerRow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(powerRow, 0, 0);
        lv_obj_set_style_pad_all(powerRow, 0, 0);
        lv_obj_set_flex_flow(powerRow, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
        createMetric(powerRow, metricWidth, portrait ? 58 : 62, "Charge", "--", &powerChargeValue_);
        createMetric(powerRow, metricWidth, portrait ? 58 : 62, "PMIC", "--", &powerPmicValue_);
        lv_obj_t *barLabel = makeLabel(body, "Battery reserve", 16);
        lv_obj_set_width(barLabel, lv_pct(100));
        powerBar_ = lv_bar_create(body);
        makeStatic(powerBar_);
        lv_obj_set_width(powerBar_, lv_pct(100));
        lv_obj_set_height(powerBar_, 14);
        lv_bar_set_range(powerBar_, 0, 100);
        lv_bar_set_value(powerBar_, 0, LV_ANIM_OFF);
        return;
    }

    if (feature == FACTORY_FEATURE_SD) {
        sdTestLabel_ = makeLabel(body, "Not tested", 18);
        lv_obj_set_width(sdTestLabel_, lv_pct(100));
    }

    if (feature == FACTORY_FEATURE_AUDIO) {
        const uint16_t buttonWidth = snapshot.width >= 500 ? 220 : (snapshot.width - 24) / 2;
        lv_obj_t *buttonRow = lv_obj_create(body);
        makeStatic(buttonRow);
        lv_obj_set_width(buttonRow, lv_pct(100));
        lv_obj_set_height(buttonRow, 46);
        lv_obj_set_style_bg_opa(buttonRow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(buttonRow, 0, 0);
        lv_obj_set_style_pad_all(buttonRow, 0, 0);
        lv_obj_set_style_pad_column(buttonRow, 8, 0);
        lv_obj_set_flex_flow(buttonRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(buttonRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        createActionButton(buttonRow, "PLAY MP3", kActionShieldBase + 17, kGreen, buttonWidth, 42);
        createActionButton(buttonRow, "BEEP", kActionShieldBase + 16, kAmber, buttonWidth, 42);
        lv_obj_t *volumeLabel = makeLabel(body, "OUTPUT VOLUME", 15);
        lv_obj_set_width(volumeLabel, lv_pct(100));
        lv_obj_set_style_text_color(volumeLabel, kMuted, 0);
        audioVolumeSlider_ = lv_slider_create(body);
        makeStatic(audioVolumeSlider_);
        lv_obj_set_width(audioVolumeSlider_, lv_pct(100));
        lv_obj_set_height(audioVolumeSlider_, 18);
        lv_slider_set_range(audioVolumeSlider_, 0, FACTORY_MAX_AUDIO_VOLUME);
        lv_slider_set_value(audioVolumeSlider_, snapshot.audioVolume, LV_ANIM_OFF);
        lv_obj_add_event_cb(audioVolumeSlider_, audioVolumeEvent, LV_EVENT_VALUE_CHANGED, this);
        return;
    }

    if (feature == FACTORY_FEATURE_SD_MUSIC) {
        const uint16_t bodyHeight = snapshot.height > 45 ? snapshot.height - 45 : snapshot.height;
        lv_obj_set_height(body, bodyHeight);
        lv_obj_set_flex_grow(body, 0);
        sdMusicStatusLabel_ = makeLabel(body, "Open to scan", 16);
        lv_obj_set_width(sdMusicStatusLabel_, lv_pct(100));
        lv_obj_set_height(sdMusicStatusLabel_, 24);
        lv_obj_set_style_text_color(sdMusicStatusLabel_, kMuted, 0);
        sdMusicList_ = lv_obj_create(body);
        lv_obj_set_width(sdMusicList_, lv_pct(100));
        lv_obj_set_flex_grow(sdMusicList_, 1);
        lv_obj_set_style_bg_opa(sdMusicList_, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(sdMusicList_, 0, 0);
        lv_obj_set_style_pad_all(sdMusicList_, 0, 0);
        lv_obj_set_style_pad_row(sdMusicList_, 5, 0);
        lv_obj_set_flex_flow(sdMusicList_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scroll_dir(sdMusicList_, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(sdMusicList_, LV_SCROLLBAR_MODE_OFF);
        return;
    }

    if (feature == FACTORY_FEATURE_MIC) {
        lv_obj_t *micTitle = makeLabel(body, "LIVE INPUT", 15);
        lv_obj_set_width(micTitle, lv_pct(100));
        lv_obj_set_style_text_color(micTitle, kMuted, 0);
        micChart_ = lv_chart_create(body);
        lv_obj_set_width(micChart_, lv_pct(100));
        lv_obj_set_height(micChart_, portrait ? 220 : 128);
        lv_obj_set_style_bg_color(micChart_, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(micChart_, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(micChart_, 1, 0);
        lv_obj_set_style_border_color(micChart_, kBorder, 0);
        lv_obj_set_style_line_width(micChart_, 2, LV_PART_ITEMS);
        lv_obj_set_style_width(micChart_, 0, LV_PART_INDICATOR);
        lv_obj_set_style_height(micChart_, 0, LV_PART_INDICATOR);
        lv_chart_set_type(micChart_, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(micChart_, 32);
        lv_chart_set_range(micChart_, LV_CHART_AXIS_PRIMARY_Y, -512, 512);
        micSeries_ = lv_chart_add_series(micChart_, color, LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_series_color(micChart_, micSeries_, kCyan);
        lv_chart_set_update_mode(micChart_, LV_CHART_UPDATE_MODE_SHIFT);
        micLevelBar_ = lv_bar_create(body);
        makeStatic(micLevelBar_);
        lv_obj_set_width(micLevelBar_, lv_pct(100));
        lv_obj_set_height(micLevelBar_, 14);
        lv_bar_set_range(micLevelBar_, 0, 32767);
        lv_bar_set_value(micLevelBar_, 0, LV_ANIM_OFF);
        micLevelLabel_ = makeLabel(body, "LEVEL 0", 15);
        lv_obj_set_width(micLevelLabel_, lv_pct(100));
        lv_obj_set_style_text_color(micLevelLabel_, kMuted, 0);
        micSensitivityLabel_ = makeLabel(body, "SENSITIVITY 5X", 15);
        lv_obj_set_width(micSensitivityLabel_, lv_pct(100));
        lv_obj_set_style_text_color(micSensitivityLabel_, kMuted, 0);
        micSensitivitySlider_ = lv_slider_create(body);
        makeStatic(micSensitivitySlider_);
        lv_obj_set_width(micSensitivitySlider_, lv_pct(100));
        lv_obj_set_height(micSensitivitySlider_, 18);
        lv_slider_set_range(micSensitivitySlider_, 1, 10);
        lv_slider_set_value(micSensitivitySlider_, micSensitivity_, LV_ANIM_OFF);
        lv_obj_add_event_cb(micSensitivitySlider_, micSensitivityEvent, LV_EVENT_VALUE_CHANGED, this);
        return;
    }

    if (feature == FACTORY_FEATURE_LORA_TX || feature == FACTORY_FEATURE_LORA_RX) {
        // LoRa diagnostics are intentionally a single-screen workflow.
        lv_obj_set_scrollable(activePageRoot_, false);
        lv_obj_set_scrollbar_mode(activePageRoot_, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_flex_grow(body, 1);
        lv_obj_t **params = feature == FACTORY_FEATURE_LORA_TX ? &loraTxParamsLabel_ : &loraRxParamsLabel_;
        lv_obj_t **message = feature == FACTORY_FEATURE_LORA_TX ? &loraTxMessageLabel_ : &loraRxMessageLabel_;
        lv_obj_t *loraPanel = lv_obj_create(body);
        makeStatic(loraPanel);
        lv_obj_set_width(loraPanel, lv_pct(100));
        lv_obj_set_height(loraPanel, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(loraPanel, 1);
        stylePanel(loraPanel);
        lv_obj_set_style_pad_all(loraPanel, 6, 0);
        lv_obj_set_style_pad_row(loraPanel, 4, 0);
        lv_obj_set_flex_flow(loraPanel, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(loraPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_t **state = feature == FACTORY_FEATURE_LORA_TX ? &loraTxStateLabel_ : &loraRxStateLabel_;
        lv_obj_t **count = feature == FACTORY_FEATURE_LORA_TX ? &loraTxCountLabel_ : &loraRxCountLabel_;
        *params = makeLabel(loraPanel, "LoRa: --", 15);
        lv_obj_set_width(*params, lv_pct(100));
        lv_label_set_long_mode(*params, LV_LABEL_LONG_WRAP);
        lv_obj_set_height(*params, 38);
        lv_obj_set_style_text_color(*params, kMuted, 0);
        if (feature == FACTORY_FEATURE_LORA_TX) {
            *state = nullptr;
        } else {
            *state = makeLabel(loraPanel, "LISTENING", 18);
            lv_obj_set_width(*state, lv_pct(100));
            lv_obj_set_style_text_align(*state, LV_TEXT_ALIGN_CENTER, 0);
        }
        *count = makeLabel(loraPanel, feature == FACTORY_FEATURE_LORA_TX ? "Packets sent: 0" : "Packets received: 0", 15);
        lv_obj_set_width(*count, lv_pct(100));
        lv_obj_set_style_text_align(*count, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(*count, kMuted, 0);
        const bool portrait = snapshot.height > snapshot.width;
        if (feature == FACTORY_FEATURE_LORA_TX) {
            // The packet sequence is transmitted internally; Packets sent is
            // the only TX counter that needs to be shown on this page.
            *message = nullptr;
        } else {
            *message = makeLabel(loraPanel, "", portrait ? 20 : 24);
            lv_obj_set_width(*message, lv_pct(100));
            lv_obj_set_style_text_align(*message, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_long_mode(*message, LV_LABEL_LONG_WRAP);
            lv_obj_set_height(*message, portrait ? 90 : 58);
            lv_obj_set_style_text_color(*message, kText, 0);
        }
        if (feature == FACTORY_FEATURE_LORA_TX) {
            loraTxCountdownLabel_ = makeLabel(loraPanel, "", 16);
            lv_obj_set_width(loraTxCountdownLabel_, lv_pct(100));
            lv_obj_set_height(loraTxCountdownLabel_, 18);
            lv_obj_set_style_text_align(loraTxCountdownLabel_, LV_TEXT_ALIGN_CENTER, 0);
            loraTxButton_ = createActionButton(loraPanel, "TRANSMIT", kActionShieldBase + 3, kAmber, 170, 42);
            lv_obj_set_style_margin_top(loraTxButton_, 10, 0);
        }
        return;
    }

    if (feature == FACTORY_FEATURE_LORA_SETTINGS) {
        loraSettingsLabel_ = makeLabel(body, "", 16);
        lv_obj_set_width(loraSettingsLabel_, lv_pct(100));
        lv_label_set_long_mode(loraSettingsLabel_, LV_LABEL_LONG_WRAP);
        lv_obj_set_height(loraSettingsLabel_, 46);
        createSettingSlider(body, "Power (dBm)", -9, 22, snapshot.loraPower, kActionLoraConfigBase + 1);
        createSettingSlider(body, "Spreading factor", 5, 12, snapshot.loraSpreadingFactor, kActionLoraConfigBase + 3);
        int32_t frequencyIndex = snapshot.loraFrequency < 450 ? 0 :
                                 (snapshot.loraFrequency < 600 ? 1 :
                                  (snapshot.loraFrequency < 900 ? 2 :
                                   (snapshot.loraFrequency < 918 ? 3 :
                                    (snapshot.loraFrequency < 922 ? 4 : 5))));
        createSettingSlider(body, "Frequency: 433 / 470 / 868 / 915 / 920 / 923 MHz",
                            0, 5, frequencyIndex, kActionLoraConfigBase + 4);
        createSettingSlider(body, "Coding rate 4/x", 5, 8, snapshot.loraCodingRate, kActionLoraConfigBase + 5);
        createSettingSlider(body, "Bandwidth: 7.8 / 15.6 / 31.25 / 62.5 / 125 / 250 / 500 kHz",
                            0, 6, snapshot.loraBandwidth <= 7.8f ? 0 :
                            (snapshot.loraBandwidth <= 15.6f ? 1 :
                             (snapshot.loraBandwidth <= 31.25f ? 2 :
                              (snapshot.loraBandwidth <= 62.5f ? 3 :
                               (snapshot.loraBandwidth <= 125.0f ? 4 :
                                (snapshot.loraBandwidth <= 250.0f ? 5 : 6))))),
                            kActionLoraConfigBase + 2);
        createSettingSlider(body, "TX interval (100 ms)", 1, 100, snapshot.loraTxIntervalMs / 100, kActionLoraConfigBase + 6);
        createSettingSlider(body, "Preamble symbols", 6, 64, snapshot.loraPreambleLength, kActionLoraConfigBase + 7);
        createSettingSlider(body, "Sync word", 0, 255, snapshot.loraSyncWord, kActionLoraConfigBase + 8);
        createSettingSlider(body, "CRC", 0, 1, snapshot.loraCrcEnabled ? 1 : 0, kActionLoraConfigBase + 9);
        return;
    }

    if (feature == FACTORY_FEATURE_WIFI) {
        lv_obj_t *info = lv_obj_create(body);
        makeStatic(info);
        lv_obj_set_width(info, lv_pct(100));
        lv_obj_set_height(info, 58);
        stylePanel(info);
        lv_obj_set_style_pad_all(info, 7, 0);
        wifiInfoLabel_ = makeLabel(info, "MAC --\nIP --", 16);
        lv_obj_set_width(wifiInfoLabel_, lv_pct(100));
        lv_label_set_long_mode(wifiInfoLabel_, LV_LABEL_LONG_WRAP);

        wifiStatusLabel_ = makeLabel(body, "Scanning nearby networks...", 16);
        lv_obj_set_width(wifiStatusLabel_, lv_pct(100));
        lv_obj_set_style_text_color(wifiStatusLabel_, kMuted, 0);

        lv_obj_t *list = lv_obj_create(body);
        makeStatic(list);
        lv_obj_set_width(list, lv_pct(100));
        lv_obj_set_height(list, portrait ? 260 : 170);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(list, 0, 0);
        lv_obj_set_style_pad_all(list, 0, 0);
        lv_obj_set_style_pad_row(list, 5, 0);
        lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scrollable(list, true);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
        for (uint8_t i = 0; i < FACTORY_MAX_WIFI_NETWORKS; ++i) {
            lv_obj_t *row = lv_button_create(list);
            makeStatic(row);
            lv_obj_set_width(row, lv_pct(100));
            lv_obj_set_height(row, 38);
            stylePanel(row);
            lv_obj_set_style_pad_left(row, 8, 0);
            lv_obj_set_style_pad_right(row, 8, 0);
            lv_obj_set_style_pad_column(row, 7, 0);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_user_data(row, (void *)(uintptr_t)(kActionWifiNetworkBase + i));
            lv_obj_add_event_cb(row, actionEvent, LV_EVENT_CLICKED, this);
            wifiNetworkIcons_[i] = makeLabel(row, LV_SYMBOL_WIFI, 20);
            lv_obj_set_width(wifiNetworkIcons_[i], 22);
            lv_obj_set_style_text_align(wifiNetworkIcons_[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(wifiNetworkIcons_[i], kCyan, 0);
            wifiNetworkLabels_[i] = makeLabel(row, "", 15);
            lv_obj_set_width(wifiNetworkLabels_[i], snapshot.width > 200 ? snapshot.width - 170 : 60);
            lv_label_set_long_mode(wifiNetworkLabels_[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_style_text_color(wifiNetworkLabels_[i], kText, 0);
            wifiNetworkRssiLabels_[i] = makeLabel(row, "-- dBm", 14);
            lv_obj_set_width(wifiNetworkRssiLabels_[i], 58);
            lv_obj_set_style_text_align(wifiNetworkRssiLabels_[i], LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_style_text_color(wifiNetworkRssiLabels_[i], kMuted, 0);
            lv_obj_t *signal = lv_obj_create(row);
            makeStatic(signal);
            lv_obj_set_size(signal, 30, 24);
            lv_obj_set_style_bg_opa(signal, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(signal, 0, 0);
            lv_obj_set_style_pad_all(signal, 0, 0);
            lv_obj_set_style_pad_column(signal, 2, 0);
            lv_obj_set_flex_flow(signal, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(signal, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
            for (uint8_t bar = 0; bar < 4; ++bar) {
                lv_obj_t *barObject = lv_obj_create(signal);
                makeStatic(barObject);
                lv_obj_set_size(barObject, 4, 5 + bar * 5);
                lv_obj_set_style_radius(barObject, 1, 0);
                lv_obj_set_style_bg_color(barObject, kBorder, 0);
                lv_obj_set_style_border_width(barObject, 0, 0);
                wifiNetworkSignalBars_[i][bar] = barObject;
            }
            lv_obj_set_hidden(row, true);
            wifiNetworkButtons_[i] = row;
        }
        wifiPingButton_ = createActionButton(body, "PING BAIDU.COM", kActionWifiPing, kCyan,
                                             portrait ? snapshot.width - 14 : 220, 42);
        wifiPingLabel_ = makeLabel(body, "", 15);
        lv_obj_set_width(wifiPingLabel_, lv_pct(100));
        lv_obj_set_style_text_align(wifiPingLabel_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(wifiPingLabel_, kMuted, 0);
        return;
    }

    if (feature == FACTORY_FEATURE_BLE) {
        lv_obj_set_flex_grow(body, 1);
        lv_obj_set_flex_align(body, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_t *card = lv_obj_create(body);
        makeStatic(card);
        lv_obj_set_width(card, portrait ? lv_pct(100) : 300);
        lv_obj_set_height(card, 130);
        stylePanel(card);
        lv_obj_set_style_pad_all(card, 12, 0);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_t *name = makeLabel(card, "FactoryV2 BLE", 22);
        lv_obj_set_width(name, lv_pct(100));
        lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
        bleStatusLabel_ = makeLabel(card, "Starting advertising...", 16);
        lv_obj_set_width(bleStatusLabel_, lv_pct(100));
        lv_obj_set_style_text_align(bleStatusLabel_, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(bleStatusLabel_, kMuted, 0);
        return;
    }

    if (feature == FACTORY_FEATURE_IR_RX || feature == FACTORY_FEATURE_IR_TX) {
        lv_obj_t **irLabel = feature == FACTORY_FEATURE_IR_TX ? &irTxDetailLabel_ : &irRxDetailLabel_;
        if (feature == FACTORY_FEATURE_IR_TX) {
            lv_obj_set_flex_align(body, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            irTxStateLabel_ = makeLabel(body, "", 20);
            irTxCountLabel_ = makeLabel(body, "Frames sent: 0", 15);
            lv_obj_set_style_text_color(irTxCountLabel_, kMuted, 0);
        }
        *irLabel = makeLabel(body, feature == FACTORY_FEATURE_IR_TX ? "NEC 0x20DF10EF" : "Listening...", 18);
        lv_obj_set_width(*irLabel, lv_pct(100));
        lv_obj_set_style_text_align(*irLabel, LV_TEXT_ALIGN_CENTER, 0);
        if (feature == FACTORY_FEATURE_IR_RX) {
            irRxProtocolLabel_ = *irLabel;
            irRxRawLabel_ = makeLabel(body, "RAW TIMINGS (HEX)\n--", 16);
            lv_obj_set_width(irRxRawLabel_, lv_pct(100));
            lv_label_set_long_mode(*irLabel, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_align(irRxRawLabel_, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_height(irRxRawLabel_, 110);
        }
        if (feature == FACTORY_FEATURE_IR_TX) {
            irTxButton_ = createActionButton(body, "START TX", kActionShieldBase + 5, kAmber, 180, 52);
        }
        return;
    }

    if (feature == FACTORY_FEATURE_IMU) {
        lv_obj_set_scrollable(activePageRoot_, false);
        lv_obj_set_scrollbar_mode(activePageRoot_, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_flex_grow(body, 1);

        lv_obj_t *levelRow = lv_obj_create(body);
        makeStatic(levelRow);
        lv_obj_set_width(levelRow, lv_pct(100));
        lv_obj_set_height(levelRow, 26);
        lv_obj_set_style_bg_opa(levelRow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(levelRow, 0, 0);
        lv_obj_set_style_pad_all(levelRow, 0, 0);
        lv_obj_set_style_pad_column(levelRow, 7, 0);
        lv_obj_set_flex_flow(levelRow, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(levelRow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        imuLevelLed_ = createStatusLed(levelRow);
        imuLevelLabel_ = makeLabel(levelRow, "TILTED", 15);
        lv_obj_set_style_text_color(imuLevelLabel_, kMuted, 0);
        imuValueLabel_ = makeLabel(levelRow, "X --  Y --  Z --", 15);
        lv_obj_set_flex_grow(imuValueLabel_, 1);
        lv_obj_set_style_text_align(imuValueLabel_, LV_TEXT_ALIGN_RIGHT, 0);

        imuChart_ = lv_chart_create(body);
        lv_obj_set_width(imuChart_, lv_pct(100));
        lv_obj_set_height(imuChart_, snapshot.height > snapshot.width ? 300 : 104);
        lv_obj_set_flex_grow(imuChart_, 1);
        lv_obj_set_style_bg_color(imuChart_, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(imuChart_, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(imuChart_, 1, 0);
        lv_obj_set_style_border_color(imuChart_, kBorder, 0);
        lv_obj_set_style_line_width(imuChart_, 2, LV_PART_ITEMS);
        lv_obj_set_style_width(imuChart_, 0, LV_PART_INDICATOR);
        lv_obj_set_style_height(imuChart_, 0, LV_PART_INDICATOR);
        lv_chart_set_type(imuChart_, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(imuChart_, 64);
        lv_chart_set_div_line_count(imuChart_, 5, 8);
        lv_chart_set_range(imuChart_, LV_CHART_AXIS_PRIMARY_Y, -2000, 2000);
        lv_chart_set_update_mode(imuChart_, LV_CHART_UPDATE_MODE_SHIFT);
        imuSeriesX_ = lv_chart_add_series(imuChart_, kRed, LV_CHART_AXIS_PRIMARY_Y);
        imuSeriesY_ = lv_chart_add_series(imuChart_, kGreen, LV_CHART_AXIS_PRIMARY_Y);
        imuSeriesZ_ = lv_chart_add_series(imuChart_, kCyan, LV_CHART_AXIS_PRIMARY_Y);

        lv_obj_t *legend = lv_obj_create(body);
        makeStatic(legend);
        lv_obj_set_width(legend, lv_pct(100));
        lv_obj_set_height(legend, portrait ? 40 : 20);
        lv_obj_set_style_bg_opa(legend, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(legend, 0, 0);
        lv_obj_set_style_pad_all(legend, 0, 0);
        lv_obj_set_flex_flow(legend, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(legend, portrait ? LV_FLEX_ALIGN_CENTER : LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_t *axisContainer = legend;
        if (portrait) {
            axisContainer = lv_obj_create(legend);
            makeStatic(axisContainer);
            lv_obj_set_width(axisContainer, lv_pct(100));
            lv_obj_set_height(axisContainer, 18);
            lv_obj_set_style_bg_opa(axisContainer, LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_width(axisContainer, 0, 0);
            lv_obj_set_style_pad_all(axisContainer, 0, 0);
            lv_obj_set_flex_flow(axisContainer, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(axisContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        }
        lv_obj_t *axis = makeLabel(axisContainer, "X", 15);
        lv_obj_set_style_text_color(axis, kRed, 0);
        axis = makeLabel(axisContainer, "Y", 15);
        lv_obj_set_style_text_color(axis, kGreen, 0);
        axis = makeLabel(axisContainer, "Z", 15);
        lv_obj_set_style_text_color(axis, kCyan, 0);
        gyroValueLabel_ = makeLabel(legend, "GYRO -- / -- / -- dps", 15);
        lv_obj_set_width(gyroValueLabel_, portrait ? lv_pct(100) : LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(gyroValueLabel_, portrait ? 0 : 1);
        lv_obj_set_style_text_align(gyroValueLabel_, portrait ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(gyroValueLabel_, kMuted, 0);
        return;
    }

    if (feature == FACTORY_FEATURE_MAGNETOMETER) {
        lv_obj_set_scrollable(activePageRoot_, false);
        lv_obj_set_scrollbar_mode(activePageRoot_, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_flex_grow(body, 1);
        const bool portrait = snapshot.height > snapshot.width;
        const int32_t dialSize = portrait ? 154 : 126;

        lv_obj_t *compassPanel = lv_obj_create(body);
        makeStatic(compassPanel);
        lv_obj_set_width(compassPanel, lv_pct(100));
        lv_obj_set_height(compassPanel, LV_SIZE_CONTENT);
        lv_obj_set_flex_grow(compassPanel, 1);
        lv_obj_set_style_bg_opa(compassPanel, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(compassPanel, 0, 0);
        lv_obj_set_style_pad_all(compassPanel, 0, 0);
        lv_obj_set_style_pad_row(compassPanel, 6, 0);
        lv_obj_set_style_pad_column(compassPanel, 12, 0);
        lv_obj_set_flex_flow(compassPanel, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(compassPanel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *dial = lv_obj_create(compassPanel);
        makeStatic(dial);
        lv_obj_set_size(dial, dialSize, dialSize);
        lv_obj_set_style_radius(dial, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dial, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(dial, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(dial, 2, 0);
        lv_obj_set_style_border_color(dial, kBorder, 0);
        lv_obj_set_style_pad_all(dial, 0, 0);

        compassNeedlePoints_[0] = {(lv_value_precise_t)(dialSize / 2), (lv_value_precise_t)(dialSize / 2)};
        compassNeedlePoints_[1] = {(lv_value_precise_t)(dialSize / 2), 18};
        compassNeedle_ = lv_line_create(dial);
        lv_line_set_points_mutable(compassNeedle_, compassNeedlePoints_, 2);
        lv_obj_set_pos(compassNeedle_, 0, 0);
        lv_obj_set_size(compassNeedle_, dialSize, dialSize);
        lv_obj_set_style_line_width(compassNeedle_, 4, 0);
        lv_obj_set_style_line_color(compassNeedle_, kRed, 0);
        lv_obj_set_style_line_rounded(compassNeedle_, true, 0);

        lv_obj_t *cardinal = makeLabel(dial, "N", 16);
        lv_obj_align(cardinal, LV_ALIGN_TOP_MID, 0, 2);
        lv_obj_set_style_text_color(cardinal, kRed, 0);
        cardinal = makeLabel(dial, "S", 16);
        lv_obj_align(cardinal, LV_ALIGN_BOTTOM_MID, 0, -2);
        cardinal = makeLabel(dial, "W", 16);
        lv_obj_align(cardinal, LV_ALIGN_LEFT_MID, 3, 0);
        cardinal = makeLabel(dial, "E", 16);
        lv_obj_align(cardinal, LV_ALIGN_RIGHT_MID, -3, 0);
        lv_obj_t *hub = lv_obj_create(dial);
        makeStatic(hub);
        lv_obj_set_size(hub, 9, 9);
        lv_obj_center(hub);
        lv_obj_set_style_radius(hub, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(hub, kText, 0);
        lv_obj_set_style_border_width(hub, 0, 0);

        lv_obj_t *readout = lv_obj_create(compassPanel);
        makeStatic(readout);
        lv_obj_set_width(readout, portrait ? lv_pct(100) : 230);
        lv_obj_set_height(readout, portrait ? 112 : dialSize);
        lv_obj_set_style_bg_opa(readout, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(readout, 0, 0);
        lv_obj_set_style_pad_all(readout, 0, 0);
        lv_obj_set_style_pad_row(readout, 5, 0);
        lv_obj_set_flex_flow(readout, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(readout, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        compassHeadingLabel_ = makeLabel(readout, "0 deg  N", 24);
        lv_obj_set_width(compassHeadingLabel_, lv_pct(100));
        magValueLabel_ = makeLabel(readout, "X --  Y --  Z --", 15);
        lv_obj_set_width(magValueLabel_, lv_pct(100));
        lv_obj_set_style_text_color(magValueLabel_, kMuted, 0);
        magCalibrationLabel_ = makeLabel(readout, "NOT CALIBRATED", 15);
        lv_obj_set_width(magCalibrationLabel_, lv_pct(100));
        magCalibrationBar_ = lv_bar_create(readout);
        makeStatic(magCalibrationBar_);
        lv_obj_set_width(magCalibrationBar_, lv_pct(100));
        lv_obj_set_height(magCalibrationBar_, 10);
        lv_bar_set_range(magCalibrationBar_, 0, 100);
        lv_bar_set_value(magCalibrationBar_, 0, LV_ANIM_OFF);

        magCalibrationButton_ = createActionButton(body, "CALIBRATE COMPASS", kActionMagCalibrate,
                                                    kCyan, actionWidth, 38);
        lv_obj_set_align(magCalibrationButton_, LV_ALIGN_CENTER);
        return;
    }

    if (feature == FACTORY_FEATURE_EXTENSIONS) {
        if (snapshot.extensionCount == 0) {
            lv_obj_t *empty = makeLabel(body, "No extensions registered", 18);
            lv_obj_set_style_text_color(empty, kMuted, 0);
        } else {
            for (uint8_t i = 0; i < snapshot.extensionCount && i < FACTORY_MAX_EXTENSIONS; ++i) {
                lv_obj_t *row = lv_obj_create(body);
                makeStatic(row);
                lv_obj_set_width(row, lv_pct(100));
                lv_obj_set_height(row, 38);
                stylePanel(row);
                lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
                lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
                lv_obj_t *iconLabel = makeLabel(row, snapshot.extensions[i].icon, 18);
                lv_obj_set_width(iconLabel, 30);
                lv_obj_t *name = makeLabel(row, snapshot.extensions[i].title, 16);
                lv_obj_set_flex_grow(name, 1);
                extensionValueLabels_[i] = makeLabel(row, snapshot.extensions[i].value, 16);
                lv_obj_set_style_text_color(extensionValueLabels_[i], snapshot.extensions[i].available ? kGreen : kMuted, 0);
            }
        }
        return;
    }

    lv_obj_t *hero = lv_obj_create(body);
    makeStatic(hero);
    lv_obj_set_width(hero, lv_pct(100));
    lv_obj_set_height(hero, portrait ? 76 : 50);
    lv_obj_set_style_bg_opa(hero, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hero, 0, 0);
    lv_obj_set_style_pad_all(hero, 0, 0);
    lv_obj_set_flex_flow(hero, portrait ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hero, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    makeIconImage(hero, icon, portrait ? 32 : 40);
    lv_obj_t *copy = lv_obj_create(hero);
    makeStatic(copy);
    lv_obj_set_flex_grow(copy, 1);
    if (portrait) lv_obj_set_width(copy, lv_pct(100));
    lv_obj_set_height(copy, portrait ? 34 : lv_pct(100));
    lv_obj_set_style_bg_opa(copy, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(copy, 0, 0);
    lv_obj_set_style_pad_left(copy, portrait ? 0 : 10, 0);
    lv_obj_set_flex_flow(copy, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(copy, LV_FLEX_ALIGN_CENTER, portrait ? LV_FLEX_ALIGN_CENTER : LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_t *desc = makeLabel(copy, description, 16);
    lv_label_set_long_mode(desc, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc, lv_pct(100));
    lv_obj_set_style_text_align(desc, portrait ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_color(desc, kMuted, 0);


    uint32_t action = 0;
    const char *actionTitle = nullptr;
    if (feature == FACTORY_FEATURE_DISPLAY) {
        action = kActionDisplay;
        actionTitle = "CYCLE COLORS";
    } else if (feature == FACTORY_FEATURE_SD) {
        action = kActionSd;
        actionTitle = "RUN SD TEST";
    } else if (feature == FACTORY_FEATURE_RGB) {
        action = kActionRgb;
        actionTitle = "SET ORANGE";
    } else if (feature == FACTORY_FEATURE_POWEROFF) {
        action = kActionPowerOff;
        actionTitle = "POWER OFF";
    }
    if (action && actionTitle) {
        lv_obj_t *actionButton = createActionButton(body, actionTitle, action, color, actionWidth, 38);
        lv_obj_set_width(actionButton, lv_pct(100));
    }

    if (feature == FACTORY_FEATURE_DISPLAY) {
        lv_obj_t *brightnessLabel = makeLabel(body, "BRIGHTNESS", 15);
        lv_obj_set_width(brightnessLabel, lv_pct(100));
        lv_obj_set_style_text_color(brightnessLabel, kMuted, 0);
        lv_obj_set_style_margin_top(brightnessLabel, 14, 0);
        lv_obj_t *brightness = lv_slider_create(body);
        makeStatic(brightness);
        lv_obj_set_width(brightness, lv_pct(100));
        lv_obj_set_height(brightness, 18);
        lv_obj_set_style_margin_top(brightness, 5, 0);
        lv_obj_set_style_margin_bottom(brightness, 10, 0);
        lv_slider_set_range(brightness, 0, 255);
        lv_slider_set_value(brightness, snapshot.brightness, LV_ANIM_OFF);
        lv_obj_add_event_cb(brightness, brightnessEvent, LV_EVENT_VALUE_CHANGED, this);
    }
    if (feature != FACTORY_FEATURE_AUDIO && feature != FACTORY_FEATURE_MIC &&
        feature != FACTORY_FEATURE_SD &&
        feature != FACTORY_FEATURE_LORA_TX && feature != FACTORY_FEATURE_LORA_RX &&
        feature != FACTORY_FEATURE_LORA_SETTINGS && feature != FACTORY_FEATURE_IR_TX &&
        feature != FACTORY_FEATURE_IR_RX) {
        featureMessage_[feature] = makeLabel(body, "", 16);
        lv_obj_set_width(featureMessage_[feature], lv_pct(100));
        lv_obj_set_style_text_color(featureMessage_[feature], kMuted, 0);
    }
}

void FactoryUi::addFeature(const FactorySnapshot &snapshot, FactoryFeature feature,
                           const lv_img_dsc_t *icon, const char *title, const char *description,
                           lv_color_t color)
{
    if (!featureAvailable(snapshot, feature)) return;
    const uint8_t page = featureCount_ + 1;
    ++featureCount_;
    pageFeatures_[page] = feature;
    createFeaturePage(snapshot, feature, page, icon, title, description, color);
    lv_obj_t *tile = createFeatureTile(selector_, icon, title, page, color,
                                       launcherTileWidth_, launcherTileHeight_);
    if (feature == FACTORY_FEATURE_SD_MUSIC) {
        lv_obj_t *badge = makeLabel(tile, LV_SYMBOL_AUDIO, 18);
        lv_obj_set_floating(badge, true);
        lv_obj_set_size(badge, 20, 20);
        lv_obj_set_style_bg_color(badge, kPanelPressed, 0);
        lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_text_align(badge, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(badge, kCyan, 0);
        lv_obj_align(badge, LV_ALIGN_CENTER, 13, -14);
    }
}

void FactoryUi::createUi(const FactorySnapshot &snapshot)
{
    featureCount_ = 0;
    const uint16_t screenWidth = snapshot.width;
    const uint16_t screenHeight = snapshot.height;
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, kBackground, 0);
    tileview_ = lv_tileview_create(screen);
    lv_obj_set_size(tileview_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(tileview_, kBackground, 0);
    lv_obj_set_scrollbar_mode(tileview_, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(tileview_, tileChangedEvent, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t *home = createPage(tileview_, &factory_icon_factory, "Factory", kHomePage, screenWidth, screenHeight);
    lv_obj_set_flex_flow(home, LV_FLEX_FLOW_COLUMN);
    lv_obj_t *welcome = makeLabel(home, snapshot.boardName, 18);
    lv_obj_set_style_text_color(welcome, kMuted, 0);
    lv_obj_set_width(welcome, lv_pct(100));
    lv_obj_set_style_text_align(welcome, LV_TEXT_ALIGN_CENTER, 0);

    selector_ = lv_obj_create(home);
    makeStatic(selector_);
    lv_obj_set_width(selector_, lv_pct(100));
    lv_obj_set_flex_grow(selector_, 1);
    lv_obj_set_style_bg_opa(selector_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(selector_, 0, 0);
    lv_obj_set_style_pad_left(selector_, 3, 0);
    lv_obj_set_style_pad_right(selector_, 3, 0);
    lv_obj_set_style_pad_top(selector_, 5, 0);
    lv_obj_set_style_pad_bottom(selector_, 5, 0);
    const bool vertical = screenHeight > screenWidth;
    lv_obj_set_style_pad_column(selector_, vertical ? 0 : 7, 0);
    lv_obj_set_style_pad_row(selector_, vertical ? 7 : 0, 0);
    lv_obj_set_flex_flow(selector_, vertical ? LV_FLEX_FLOW_COLUMN : LV_FLEX_FLOW_ROW);
    // A vertically overflowing flex list must start at the top. Centering the
    // main axis makes LVGL distribute the overflow around the viewport, so
    // dragging to the first item snaps back toward the middle of the list.
    lv_obj_set_flex_align(selector_, LV_FLEX_ALIGN_START,
                          vertical ? LV_FLEX_ALIGN_START : LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollable(selector_, true);
    lv_obj_set_scroll_dir(selector_, vertical ? LV_DIR_VER : LV_DIR_HOR);
    if (vertical) lv_obj_set_scroll_snap_y(selector_, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(selector_, LV_SCROLLBAR_MODE_OFF);

    launcherTileWidth_ = vertical ? screenWidth - 20 : (screenWidth >= 500 ? 104 : 92);
    launcherTileHeight_ = vertical ? 84 : 104;

    addFeature(snapshot, FACTORY_FEATURE_DISPLAY, &factory_icon_display, "Display", "Panel and color cycle", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_SD, &factory_icon_sim_card, "SD card", "Mount and card check", kAmber);
    addFeature(snapshot, FACTORY_FEATURE_RGB, &factory_icon_flash_on, "RGB", "Pixel output", kAmber);
    addFeature(snapshot, FACTORY_FEATURE_TOUCH, &factory_icon_touchscreen_smartphone, "Touch", "Touch coordinates", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_POWER, &factory_icon_charge_battery, "Power", "Battery and PMIC", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_POWEROFF, &factory_icon_flash_off, "Power off", "Enter shipping mode", kRed);
    addFeature(snapshot, FACTORY_FEATURE_AUDIO, &factory_icon_speaker, "Audio", "Play a tone", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_SD_MUSIC, &factory_icon_sim_card, "SD Music", "Play MP3 files", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_MIC, &factory_icon_sound_recording_copyright, "Mic", "Microphone level", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_LORA_TX, &factory_icon_upload, "LoRa TX", "Transmit packet", kAmber);
    addFeature(snapshot, FACTORY_FEATURE_LORA_RX, &factory_icon_download, "LoRa RX", "Receive packet", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_LORA_SETTINGS, &factory_icon_settings, "LoRa setup", "Radio parameters", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_IR_TX, &factory_icon_export, "IR TX", "Transmit NEC", kAmber);
    addFeature(snapshot, FACTORY_FEATURE_IR_RX, &factory_icon_import, "IR RX", "Decode NEC", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_IMU, &factory_icon_electrical_sensor, "IMU", "Motion and level", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_MAGNETOMETER, &factory_icon_radar_plot, "Compass", "Calibrate and heading", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_LIVE, &factory_icon_timeline, "Live", "Board telemetry", kGreen);
    addFeature(snapshot, FACTORY_FEATURE_INFO, &factory_icon_factory, "Info", "Hardware diagnostics", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_WIFI, &factory_icon_wifi, "Wi-Fi", "Scan and connect", kCyan);
    addFeature(snapshot, FACTORY_FEATURE_BLE, &factory_icon_electronics, "BLE", "Advertise beacon", kGreen);
    if (snapshot.extensionCount > 0) {
        addFeature(snapshot, FACTORY_FEATURE_EXTENSIONS, &factory_icon_grid, "More", "Extension modules", kCyan);
    }
    createActionTile(selector_, &factory_icon_rotate, "Rotate", kActionRotation, kCyan,
                     launcherTileWidth_, launcherTileHeight_);

    lv_obj_t *footer = makeLabel(home, vertical ? "Swipe up or down to select" : "Swipe left or right to select", 16);
    lv_obj_set_width(footer, lv_pct(100));
    lv_obj_set_style_text_align(footer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(footer, kMuted, 0);
}

void FactoryUi::begin(const FactorySnapshot &snapshot, const FactoryActions &actions)
{
    actions_ = actions;
    createUi(snapshot);
}

void FactoryUi::rebuild(const FactorySnapshot &snapshot)
{
    if (colorTestOverlay_) {
        lv_obj_del(colorTestOverlay_);
        colorTestOverlay_ = nullptr;
    }
    if (powerOffDialog_) {
        lv_obj_del(powerOffDialog_);
        powerOffDialog_ = nullptr;
    }
    lv_obj_clean(lv_screen_active());
    tileview_ = nullptr;
    activePageRoot_ = nullptr;
    selector_ = nullptr;
    timeLabel_ = nullptr;
    uptimeLabel_ = nullptr;
    touchLabel_ = nullptr;
    powerLabel_ = nullptr;
    sensorLabel_ = nullptr;
    rtcLabel_ = nullptr;
    sdLabel_ = nullptr;
    memoryLabel_ = nullptr;
    sdTestLabel_ = nullptr;
    powerBatteryValue_ = nullptr;
    powerVbusValue_ = nullptr;
    powerChargeValue_ = nullptr;
    powerPmicValue_ = nullptr;
    powerBar_ = nullptr;
    touchCoordinateLabel_ = nullptr;
    audioVolumeSlider_ = nullptr;
    sdMusicStatusLabel_ = nullptr;
    sdMusicList_ = nullptr;
    memset(sdMusicButtons_, 0, sizeof(sdMusicButtons_));
    micLevelBar_ = nullptr;
    micLevelLabel_ = nullptr;
    micSensitivitySlider_ = nullptr;
    micSensitivityLabel_ = nullptr;
    loraTxParamsLabel_ = nullptr;
    loraRxParamsLabel_ = nullptr;
    loraTxMessageLabel_ = nullptr;
    loraTxCountdownLabel_ = nullptr;
    loraTxButton_ = nullptr;
    loraRxMessageLabel_ = nullptr;
    loraTxStateLabel_ = nullptr;
    loraTxCountLabel_ = nullptr;
    loraRxStateLabel_ = nullptr;
    loraRxCountLabel_ = nullptr;
    loraSettingsLabel_ = nullptr;
    irTxDetailLabel_ = nullptr;
    irRxDetailLabel_ = nullptr;
    irTxIndicator_ = nullptr;
    irTxStateLabel_ = nullptr;
    irTxCountLabel_ = nullptr;
    irTxButton_ = nullptr;
    irRxProtocolLabel_ = nullptr;
    irRxRawLabel_ = nullptr;
    imuChart_ = nullptr;
    imuSeriesX_ = nullptr;
    imuSeriesY_ = nullptr;
    imuSeriesZ_ = nullptr;
    imuLevelLed_ = nullptr;
    imuLevelLabel_ = nullptr;
    imuValueLabel_ = nullptr;
    gyroValueLabel_ = nullptr;
    compassNeedle_ = nullptr;
    compassHeadingLabel_ = nullptr;
    wifiStatusLabel_ = nullptr;
    wifiInfoLabel_ = nullptr;
    wifiPingLabel_ = nullptr;
    wifiPingButton_ = nullptr;
    bleStatusLabel_ = nullptr;
    memset(wifiNetworkButtons_, 0, sizeof(wifiNetworkButtons_));
    memset(wifiNetworkLabels_, 0, sizeof(wifiNetworkLabels_));
    memset(wifiNetworkRssiLabels_, 0, sizeof(wifiNetworkRssiLabels_));
    memset(wifiNetworkSignalBars_, 0, sizeof(wifiNetworkSignalBars_));
    memset(wifiNetworkIcons_, 0, sizeof(wifiNetworkIcons_));
    memset(infoStatusLabels_, 0, sizeof(infoStatusLabels_));
    lastWifiScanGeneration_ = 0;
    magValueLabel_ = nullptr;
    magCalibrationLabel_ = nullptr;
    magCalibrationBar_ = nullptr;
    magCalibrationButton_ = nullptr;
    touchCrosshairH_ = nullptr;
    touchCrosshairV_ = nullptr;
    micChart_ = nullptr;
    micSeries_ = nullptr;
    memset(featureMessage_, 0, sizeof(featureMessage_));
    memset(extensionValueLabels_, 0, sizeof(extensionValueLabels_));
    memset(pageFeatures_, 0, sizeof(pageFeatures_));
    memset(pageStatusLed_, 0, sizeof(pageStatusLed_));
    memset(pageTimeLabels_, 0, sizeof(pageTimeLabels_));
    lastStatusTime_[0] = '\0';
    featureCount_ = 0;
    activePage_ = kHomePage;
    lastLoraTxCount_ = 0;
    lastIrTxCount_ = 0;
    lastImuChartMs_ = 0;
    lastSdMusicGeneration_ = UINT32_MAX;
    createUi(snapshot);
}

void FactoryUi::goHome()
{
    if (colorTestOverlay_) {
        lv_obj_del(colorTestOverlay_);
        colorTestOverlay_ = nullptr;
    }
    if (powerOffDialog_) {
        lv_obj_del(powerOffDialog_);
        powerOffDialog_ = nullptr;
    }
    if (!tileview_) return;
    if (actions_.shieldStop) actions_.shieldStop(actions_.context);
    if (actions_.bleSetActive) actions_.bleSetActive(actions_.context, false);
    activePage_ = kHomePage;
    lv_obj_set_tile_id(tileview_, kHomePage, 0, LV_ANIM_OFF);
}

void FactoryUi::update(const FactorySnapshot &snapshot)
{
    char text[160];
    if (strcmp(lastStatusTime_, snapshot.statusTime) != 0) {
        snprintf(lastStatusTime_, sizeof(lastStatusTime_), "%s", snapshot.statusTime);
        for (uint8_t page = 0; page <= featureCount_; ++page) {
            if (pageTimeLabels_[page]) lv_label_set_text(pageTimeLabels_[page], snapshot.statusTime);
        }
    }
    if (timeLabel_) {
        lv_label_set_text(timeLabel_, snapshot.clock);
        snprintf(text, sizeof(text), "%lus", (unsigned long)(millis() / 1000));
        lv_label_set_text(uptimeLabel_, text);
        snprintf(text, sizeof(text), "%s  X:%d Y:%d", snapshot.touched ? "PRESS" : "IDLE", snapshot.touchX, snapshot.touchY);
        lv_label_set_text(touchLabel_, snapshot.hasTouch ? text : "N/A");
        if (snapshot.hasPmic) snprintf(text, sizeof(text), "%umV / %s", snapshot.batteryMillivolts, snapshot.chargeStatus);
        else snprintf(text, sizeof(text), "N/A");
        lv_label_set_text(powerLabel_, text);
        snprintf(text, sizeof(text), "%.1f C / %.1f lx", snapshot.coreTemperature, snapshot.lightLux);
        lv_label_set_text(sensorLabel_, snapshot.hasLightSensor ? text : "Core only");
        lv_label_set_text(rtcLabel_, snapshot.rtc);
        lv_label_set_text(sdLabel_, snapshot.sd);
        snprintf(text, sizeof(text), "%luK heap / %luK PSRAM", (unsigned long)(snapshot.freeHeap / 1024), (unsigned long)(snapshot.freePsram / 1024));
        lv_label_set_text(memoryLabel_, text);
    }
    if (sdTestLabel_) {
        snprintf(text, sizeof(text), "%s\nMount: %lums  Capacity: %luMB\nRead: %luKB/s  Write: %luKB/s\nProgress: %lu%%",
                 snapshot.sdTestStatus[0] ? snapshot.sdTestStatus : (snapshot.sdReady ? "Complete" : "Not tested"),
                 (unsigned long)snapshot.sdMountMs, (unsigned long)snapshot.sdCapacityMb,
                 (unsigned long)snapshot.sdReadKbps, (unsigned long)snapshot.sdWriteKbps,
                 (unsigned long)snapshot.sdTestProgress);
        lv_label_set_text(sdTestLabel_, text);
    }
    const bool touchPageActive = activePage_ > 0 && activePage_ <= featureCount_ &&
                                 pageFeatures_[activePage_] == FACTORY_FEATURE_TOUCH;
    if (touchCrosshairH_ && touchCrosshairV_ && snapshot.hasTouch) {
        const bool showCrosshair = touchPageActive && snapshot.touched;
        lv_obj_set_hidden(touchCrosshairH_, !showCrosshair);
        lv_obj_set_hidden(touchCrosshairV_, !showCrosshair);
        if (showCrosshair) {
            lv_obj_set_y(touchCrosshairH_, snapshot.touchY - 1);
            lv_obj_set_x(touchCrosshairV_, snapshot.touchX - 1);
        }
    }
    if (touchCoordinateLabel_) {
        const bool showCoordinates = touchPageActive && snapshot.touched;
        lv_obj_set_hidden(touchCoordinateLabel_, !showCoordinates);
        if (showCoordinates) {
            snprintf(text, sizeof(text), "X %d   Y %d", snapshot.touchX, snapshot.touchY);
            lv_label_set_text(touchCoordinateLabel_, text);
        }
    }
    if (powerBatteryValue_) {
        snprintf(text, sizeof(text), "%umV", snapshot.batteryMillivolts);
        lv_label_set_text(powerBatteryValue_, text);
    }
    if (powerVbusValue_) {
        snprintf(text, sizeof(text), "%umV", snapshot.vbusMillivolts);
        lv_label_set_text(powerVbusValue_, text);
    }
    if (powerChargeValue_) lv_label_set_text(powerChargeValue_, snapshot.chargeStatus);
    if (powerPmicValue_) lv_label_set_text(powerPmicValue_, snapshot.hasPmic ? snapshot.pmicName : "N/A");
    if (powerBar_) {
        const int reserve = constrain(((int)snapshot.batteryMillivolts - 3300) * 100 / 900, 0, 100);
        lv_bar_set_value(powerBar_, reserve, LV_ANIM_OFF);
    }
    if (audioVolumeSlider_) lv_slider_set_value(audioVolumeSlider_, snapshot.audioVolume, LV_ANIM_OFF);
    if (sdMusicStatusLabel_) {
        lv_label_set_text(sdMusicStatusLabel_, snapshot.sdMusicStatus[0] ? snapshot.sdMusicStatus : "Open to scan");
        lv_obj_set_style_text_color(sdMusicStatusLabel_, snapshot.sdMusicPlaying ? kGreen :
                                    (snapshot.audioFault && snapshot.sdMusicCurrentPath[0] ? kRed : kMuted), 0);
    }
    if (sdMusicList_ && lastSdMusicGeneration_ != snapshot.sdMusicGeneration) {
        lastSdMusicGeneration_ = snapshot.sdMusicGeneration;
        lv_obj_clean(sdMusicList_);
        memset(sdMusicButtons_, 0, sizeof(sdMusicButtons_));
        if (!snapshot.sdMusicScanning && snapshot.sdMusicTrackCount == 0) {
            lv_obj_t *empty = makeLabel(sdMusicList_, "No MP3 files found", 18);
            lv_obj_set_width(empty, lv_pct(100));
            lv_obj_set_style_text_align(empty, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(empty, kMuted, 0);
        }
        for (uint8_t i = 0; i < snapshot.sdMusicTrackCount; ++i) {
            const FactorySdMusicTrack &track = snapshot.sdMusicTracks[i];
            lv_obj_t *button = lv_button_create(sdMusicList_);
            sdMusicButtons_[i] = button;
            lv_obj_set_width(button, lv_pct(100));
            lv_obj_set_height(button, 44);
            lv_obj_set_style_bg_color(button, kPanel, 0);
            lv_obj_set_style_bg_color(button, kPanelPressed, LV_STATE_PRESSED);
            lv_obj_set_style_border_width(button, 1, 0);
            lv_obj_set_style_border_color(button, kBorder, 0);
            lv_obj_set_style_radius(button, 6, 0);
            lv_obj_set_style_pad_left(button, 10, 0);
            lv_obj_set_style_pad_right(button, 10, 0);
            lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_user_data(button, (void *)(uintptr_t)(kActionSdMusicTrackBase + i));
            lv_obj_add_event_cb(button, actionEvent, LV_EVENT_CLICKED, this);
            lv_obj_t *play = makeLabel(button, LV_SYMBOL_PLAY, 18);
            lv_obj_set_width(play, 24);
            lv_obj_set_style_text_color(play, kCyan, 0);
            lv_obj_t *name = makeLabel(button, track.name, 16);
            lv_obj_set_flex_grow(name, 1);
            lv_label_set_long_mode(name, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_t *size = makeLabel(button, "", 16);
            lv_obj_set_width(size, 68);
            lv_obj_set_style_text_align(size, LV_TEXT_ALIGN_RIGHT, 0);
            lv_obj_set_style_text_color(size, kMuted, 0);
            snprintf(text, sizeof(text), "%.1f MB", track.sizeBytes / (1024.0f * 1024.0f));
            lv_label_set_text(size, text);
        }
    }
    if (sdMusicList_) {
        for (uint8_t i = 0; i < snapshot.sdMusicTrackCount; ++i) {
            if (!sdMusicButtons_[i]) continue;
            const bool selected = snapshot.sdMusicCurrentPath[0] &&
                                  strcmp(snapshot.sdMusicCurrentPath, snapshot.sdMusicTracks[i].path) == 0;
            lv_obj_set_style_border_color(sdMusicButtons_[i], selected ? kCyan : kBorder, 0);
            lv_obj_set_style_bg_color(sdMusicButtons_[i], selected ? kPanelPressed : kPanel, 0);
        }
    }
    if (micLevelBar_) lv_bar_set_value(micLevelBar_, constrain((int)snapshot.micLevel, 0, 32767), LV_ANIM_OFF);
    if (micLevelLabel_) {
        snprintf(text, sizeof(text), "LEVEL %d  %s", snapshot.micLevel,
                 snapshot.micActive ? "SAMPLING" : (snapshot.micFault ? "FAULT" : "IDLE"));
        lv_label_set_text(micLevelLabel_, text);
        lv_obj_set_style_text_color(micLevelLabel_, snapshot.micFault ? kRed : kMuted, 0);
    }
    if (micSensitivitySlider_) lv_slider_set_value(micSensitivitySlider_, micSensitivity_, LV_ANIM_OFF);
    if (micSensitivityLabel_) {
        snprintf(text, sizeof(text), "SENSITIVITY %ldX", (long)micSensitivity_);
        lv_label_set_text(micSensitivityLabel_, text);
    }
    if (micChart_ && micSeries_) {
        int32_t waveform[32];
        for (uint8_t i = 0; i < 32; ++i) {
            waveform[i] = constrain((int32_t)snapshot.micWaveform[i] * micSensitivity_ / 5, -512, 512);
        }
        lv_chart_set_series_values(micChart_, micSeries_, waveform, 32);
        lv_chart_refresh(micChart_);
    }
    if (loraTxParamsLabel_ || loraRxParamsLabel_ || loraSettingsLabel_) {
        snprintf(text, sizeof(text), "%.1f MHz  BW %.2f kHz  SF%u  CR 4/%u  PWR %d dBm  TX %lu ms  PRE %u  SW 0x%02X  CRC %s",
                 snapshot.loraFrequency, snapshot.loraBandwidth, snapshot.loraSpreadingFactor,
                 snapshot.loraCodingRate, snapshot.loraPower, (unsigned long)snapshot.loraTxIntervalMs,
                 snapshot.loraPreambleLength, snapshot.loraSyncWord, snapshot.loraCrcEnabled ? "ON" : "OFF");
        if (loraTxParamsLabel_) lv_label_set_text(loraTxParamsLabel_, text);
        if (loraRxParamsLabel_) lv_label_set_text(loraRxParamsLabel_, text);
        if (loraSettingsLabel_) lv_label_set_text(loraSettingsLabel_, text);
    }
    if (loraTxStateLabel_) {
        lv_label_set_text(loraTxStateLabel_, snapshot.loraFault ? "FAULT" :
                          (snapshot.loraTxActive ? "TRANSMITTING" : ""));
        lv_obj_set_style_text_color(loraTxStateLabel_, snapshot.loraFault ? kRed : kText, 0);
    }
    if (loraTxCountLabel_) {
        snprintf(text, sizeof(text), "Packets sent: %lu", (unsigned long)snapshot.loraTxCount);
        lv_label_set_text(loraTxCountLabel_, text);
    }
    if (loraRxStateLabel_) {
        lv_label_set_text(loraRxStateLabel_, snapshot.loraFault ? "FAULT" :
                          (snapshot.loraRxActive ? "LISTENING" : "IDLE"));
        lv_obj_set_style_text_color(loraRxStateLabel_, snapshot.loraFault ? kRed : kText, 0);
    }
    if (loraRxCountLabel_) {
        snprintf(text, sizeof(text), "Packets received: %lu", (unsigned long)snapshot.loraRxCount);
        lv_label_set_text(loraRxCountLabel_, text);
    }
    if (loraTxMessageLabel_) lv_label_set_text(loraTxMessageLabel_, snapshot.loraTxMessage);
    if (loraTxCountdownLabel_) {
        if (snapshot.loraTxActive) {
            lv_label_set_text(loraTxCountdownLabel_, "TRANSMITTING");
        } else if (snapshot.loraTxCountdownMs) {
            snprintf(text, sizeof(text), "NEXT TX IN %.1fs", snapshot.loraTxCountdownMs / 1000.0f);
            lv_label_set_text(loraTxCountdownLabel_, text);
        } else {
            lv_label_set_text(loraTxCountdownLabel_, snapshot.loraFault ? "TX FAULT" : "");
        }
    }
    if (loraTxButton_ && snapshot.loraTxCount != lastLoraTxCount_) {
        if (lastLoraTxCount_ != 0) {
            lv_anim_t animation;
            lv_anim_init(&animation);
            lv_anim_set_var(&animation, loraTxButton_);
            lv_anim_set_exec_cb(&animation, pulseScaleEvent);
            lv_anim_set_values(&animation, 256, 286);
            lv_anim_set_duration(&animation, 100);
            lv_anim_set_playback_duration(&animation, 130);
            lv_anim_start(&animation);
        }
        lastLoraTxCount_ = snapshot.loraTxCount;
    }
    if (loraRxMessageLabel_) {
        if (snapshot.loraRxMessage[0]) {
            snprintf(text, sizeof(text), "%s\nRSSI %d dBm  SNR %.1f dB", snapshot.loraRxMessage,
                     snapshot.loraRssi, snapshot.loraSnr);
        } else {
            text[0] = '\0';
        }
        lv_label_set_text(loraRxMessageLabel_, text);
    }
    if (irRxDetailLabel_) {
        if (snapshot.irIsNec) {
            snprintf(text, sizeof(text), "NEC 0x%08lX (%u bit)",
                     (unsigned long)snapshot.shieldIrCode, snapshot.irBits);
        } else if (snapshot.irRaw[0]) {
            snprintf(text, sizeof(text), "NOT NEC - raw signal");
        } else {
            snprintf(text, sizeof(text), "Listening...");
        }
        lv_label_set_text(irRxDetailLabel_, text);
        lv_obj_set_style_text_color(irRxDetailLabel_, snapshot.irRaw[0] && !snapshot.irIsNec ? kRed :
                                    (snapshot.irIsNec ? kGreen : kMuted), 0);
    }
    if (irRxRawLabel_) {
        snprintf(text, sizeof(text), "RAW TIMINGS (HEX)\n%s", snapshot.irRaw[0] ? snapshot.irRaw : "--");
        lv_label_set_text(irRxRawLabel_, text);
    }
    if (irTxIndicator_) {
        lv_obj_set_style_bg_color(irTxIndicator_, snapshot.irFault ? kRed :
                                  (snapshot.irTxActive ? kCyan : kGreen), 0);
        lv_obj_set_style_border_color(irTxIndicator_, snapshot.irFault ? kRed : kGreen, 0);
    }
    if (irTxStateLabel_) {
        lv_label_set_text(irTxStateLabel_, snapshot.irFault ? "FAULT" :
                          (snapshot.irTxActive ? "TRANSMITTING" :
                           (snapshot.irTxRunning ? "RUNNING - 1 FRAME/S" : "STOPPED")));
        lv_obj_set_style_text_color(irTxStateLabel_, snapshot.irFault ? kRed : kText, 0);
    }
    if (irTxCountLabel_) {
        snprintf(text, sizeof(text), "Frames sent: %lu", (unsigned long)snapshot.irTxCount);
        lv_label_set_text(irTxCountLabel_, text);
    }
    if (irTxButton_) {
        lv_obj_t *label = lv_obj_get_child(irTxButton_, 0);
        if (label) lv_label_set_text(label, snapshot.irTxRunning ? "STOP TX" : "START TX");
        lv_obj_set_style_bg_color(irTxButton_, snapshot.irTxRunning ? kRed : kAmber, 0);
    }
    if (irTxButton_ && snapshot.irTxCount != lastIrTxCount_) {
        if (lastIrTxCount_ != 0) {
            lv_anim_t animation;
            lv_anim_init(&animation);
            lv_anim_set_var(&animation, irTxButton_);
            lv_anim_set_exec_cb(&animation, pulseScaleEvent);
            lv_anim_set_values(&animation, 256, 286);
            lv_anim_set_duration(&animation, 100);
            lv_anim_set_playback_duration(&animation, 130);
            lv_anim_start(&animation);
        }
        lastIrTxCount_ = snapshot.irTxCount;
    }
    if (wifiInfoLabel_) {
        snprintf(text, sizeof(text), "MAC %s\nIP %s", snapshot.wifiMac[0] ? snapshot.wifiMac : "--",
                 snapshot.wifiIp[0] ? snapshot.wifiIp : "--");
        lv_label_set_text(wifiInfoLabel_, text);
    }
    if (wifiStatusLabel_) {
        lv_label_set_text(wifiStatusLabel_, snapshot.wifiScanning ? "Scanning nearby networks..." :
                          (snapshot.wifiStatus[0] ? snapshot.wifiStatus : "Wi-Fi idle"));
        lv_obj_set_style_text_color(wifiStatusLabel_, snapshot.wifiConnecting ? kAmber :
                                    (snapshot.wifiConnected ? kGreen : kMuted), 0);
    }
    if (wifiPingLabel_) {
        lv_label_set_text(wifiPingLabel_, snapshot.wifiPingRunning ? "Pinging baidu.com..." : snapshot.wifiPingStatus);
        lv_obj_set_style_text_color(wifiPingLabel_, snapshot.wifiPingOk ? kGreen : kMuted, 0);
    }
    if (snapshot.wifiScanGeneration != lastWifiScanGeneration_) {
        lastWifiScanGeneration_ = snapshot.wifiScanGeneration;
        for (uint8_t i = 0; i < FACTORY_MAX_WIFI_NETWORKS; ++i) {
            if (!wifiNetworkButtons_[i] || !wifiNetworkLabels_[i]) continue;
            const bool visible = i < snapshot.wifiNetworkCount;
            lv_obj_set_hidden(wifiNetworkButtons_[i], !visible);
            if (visible) {
                const FactoryWifiNetwork &network = snapshot.wifiNetworks[i];
                snprintf(text, sizeof(text), "%s  CH%u%s", network.ssid, (unsigned)network.channel,
                         network.encrypted ? "  *" : "");
                lv_label_set_text(wifiNetworkLabels_[i], text);
                snprintf(text, sizeof(text), "%d dBm", (int)network.rssi);
                lv_label_set_text(wifiNetworkRssiLabels_[i], text);
                const uint8_t level = network.rssi >= -55 ? 4 :
                                      (network.rssi >= -67 ? 3 : (network.rssi >= -78 ? 2 : 1));
                for (uint8_t bar = 0; bar < 4; ++bar) {
                    lv_obj_set_style_bg_color(wifiNetworkSignalBars_[i][bar], bar < level ? kGreen : kBorder, 0);
                }
            }
        }
    }
    if (bleStatusLabel_) {
        const char *bleText = snapshot.bleConnected ? "CONNECTED" :
                              (snapshot.bleActive ?
                               (strcmp(snapshot.bleStatus, "Disconnected") == 0 ? "DISCONNECTED" : "ADVERTISING") :
                               (snapshot.bleStatus[0] ? snapshot.bleStatus : "BLE idle"));
        lv_label_set_text(bleStatusLabel_, bleText);
        lv_obj_set_style_text_color(bleStatusLabel_, snapshot.bleConnected ? kGreen :
                                    (snapshot.bleActive ? kAmber : kMuted), 0);
    }
    const bool infoPassed[] = {
        snapshot.width > 0 && snapshot.height > 0,
        snapshot.hasTouch,
        snapshot.hasSd && snapshot.sdReady,
        snapshot.hasPmic,
        snapshot.hasRtc,
        snapshot.freePsram > 0,
        snapshot.hasShield,
        snapshot.shieldExpanderReady,
        snapshot.shieldLoraReady,
        snapshot.shieldCodecReady,
        snapshot.shieldMicReady,
        snapshot.shieldIrReady,
        snapshot.shieldImuReady,
        snapshot.shieldMagReady,
    };
    for (uint8_t i = 0; i < 14; ++i) {
        if (!infoStatusLabels_[i]) continue;
        lv_label_set_text(infoStatusLabels_[i], infoPassed[i] ? "PASS" : "FAIL");
        lv_obj_set_style_text_color(infoStatusLabels_[i], infoPassed[i] ? kGreen : kRed, 0);
    }
    if (imuLevelLed_) {
        lv_obj_set_style_bg_color(imuLevelLed_, snapshot.imuLevel ? kGreen : kRed, 0);
        lv_obj_set_style_border_color(imuLevelLed_, snapshot.imuLevel ? kGreen : kRed, 0);
    }
    if (imuLevelLabel_) {
        lv_label_set_text(imuLevelLabel_, snapshot.imuLevel ? "LEVEL" : "TILTED");
        lv_obj_set_style_text_color(imuLevelLabel_, snapshot.imuLevel ? kGreen : kMuted, 0);
    }
    if (imuValueLabel_) {
        snprintf(text, sizeof(text), "X %+.2f  Y %+.2f  Z %+.2f g",
                 snapshot.imuAx, snapshot.imuAy, snapshot.imuAz);
        lv_label_set_text(imuValueLabel_, text);
    }
    if (gyroValueLabel_) {
        snprintf(text, sizeof(text), "GYRO %+.1f / %+.1f / %+.1f dps",
                 snapshot.gyroGx, snapshot.gyroGy, snapshot.gyroGz);
        lv_label_set_text(gyroValueLabel_, text);
    }
    const bool imuPageActive = activePage_ > 0 && activePage_ <= featureCount_ &&
                               pageFeatures_[activePage_] == FACTORY_FEATURE_IMU;
    if (imuPageActive && imuChart_ && imuSeriesX_ && imuSeriesY_ && imuSeriesZ_ &&
        millis() - lastImuChartMs_ >= 30) {
        lastImuChartMs_ = millis();
        lv_chart_set_next_value(imuChart_, imuSeriesX_, (int32_t)(snapshot.imuAx * 1000.0f));
        lv_chart_set_next_value(imuChart_, imuSeriesY_, (int32_t)(snapshot.imuAy * 1000.0f));
        lv_chart_set_next_value(imuChart_, imuSeriesZ_, (int32_t)(snapshot.imuAz * 1000.0f));
    }
    if (compassHeadingLabel_) {
        const char *direction = "N";
        const float heading = snapshot.magHeading;
        if (heading >= 22.5f && heading < 67.5f) direction = "NE";
        else if (heading < 112.5f) direction = "E";
        else if (heading < 157.5f) direction = "SE";
        else if (heading < 202.5f) direction = "S";
        else if (heading < 247.5f) direction = "SW";
        else if (heading < 292.5f) direction = "W";
        else if (heading < 337.5f) direction = "NW";
        snprintf(text, sizeof(text), "%03d deg  %s", (int)lroundf(heading) % 360, direction);
        lv_label_set_text(compassHeadingLabel_, text);
    }
    if (magValueLabel_) {
        snprintf(text, sizeof(text), "X %+.3f  Y %+.3f  Z %+.3f G",
                 snapshot.magX, snapshot.magY, snapshot.magZ);
        lv_label_set_text(magValueLabel_, text);
    }
    if (compassNeedle_) {
        lv_obj_t *dial = lv_obj_get_parent(compassNeedle_);
        const int32_t size = lv_obj_get_width(dial);
        const int32_t center = size / 2;
        const int32_t radius = center - 22;
        const float radians = snapshot.magHeading * (float)M_PI / 180.0f;
        compassNeedlePoints_[0] = {(lv_value_precise_t)center, (lv_value_precise_t)center};
        compassNeedlePoints_[1] = {
            (lv_value_precise_t)(center + sinf(radians) * radius),
            (lv_value_precise_t)(center - cosf(radians) * radius),
        };
        lv_line_set_points_mutable(compassNeedle_, compassNeedlePoints_, 2);
        lv_obj_set_size(compassNeedle_, size, size);
    }
    if (magCalibrationBar_) {
        lv_bar_set_value(magCalibrationBar_, snapshot.magCalibrationProgress, LV_ANIM_OFF);
    }
    if (magCalibrationLabel_) {
        if (snapshot.magCalibrating) {
            snprintf(text, sizeof(text), "CALIBRATING %u%% - ROTATE ALL AXES",
                     snapshot.magCalibrationProgress);
            lv_obj_set_style_text_color(magCalibrationLabel_, kAmber, 0);
        } else if (snapshot.magNorthAlignmentPending) {
            snprintf(text, sizeof(text), "POINT TOP NORTH, THEN TAP ALIGN");
            lv_obj_set_style_text_color(magCalibrationLabel_, kAmber, 0);
        } else if (snapshot.magCalibrated) {
            snprintf(text, sizeof(text), "CALIBRATED + NORTH ALIGNED");
            lv_obj_set_style_text_color(magCalibrationLabel_, kGreen, 0);
        } else if (snapshot.magCalibrationProgress == 100) {
            snprintf(text, sizeof(text), "MOVE MORE AND RETRY");
            lv_obj_set_style_text_color(magCalibrationLabel_, kRed, 0);
        } else {
            snprintf(text, sizeof(text), "NOT CALIBRATED");
            lv_obj_set_style_text_color(magCalibrationLabel_, kMuted, 0);
        }
        lv_label_set_text(magCalibrationLabel_, text);
    }
    if (magCalibrationButton_) {
        lv_obj_t *label = lv_obj_get_child(magCalibrationButton_, 0);
        if (label) {
            lv_label_set_text(label, snapshot.magNorthAlignmentPending ? "ALIGN NORTH" :
                              (snapshot.magCalibrated ? "RECALIBRATE" : "CALIBRATE COMPASS"));
        }
    }
    for (uint8_t i = 0; i < snapshot.extensionCount && i < FACTORY_MAX_EXTENSIONS; ++i) {
        if (!extensionValueLabels_[i]) continue;
        lv_label_set_text(extensionValueLabels_[i], snapshot.extensions[i].value[0] ? snapshot.extensions[i].value : (snapshot.extensions[i].available ? "AVAILABLE" : "N/A"));
        lv_obj_set_style_text_color(extensionValueLabels_[i], snapshot.extensions[i].available ? kGreen : kMuted, 0);
    }
    for (uint8_t page = 1; page <= featureCount_; ++page) {
        FactoryFeature feature = pageFeatures_[page];
        bool ready = featureAvailable(snapshot, feature);
        bool fault = false;
        switch (feature) {
        case FACTORY_FEATURE_AUDIO: fault = snapshot.audioFault; break;
        case FACTORY_FEATURE_SD_MUSIC: fault = snapshot.audioFault; break;
        case FACTORY_FEATURE_MIC: fault = snapshot.micFault; break;
        case FACTORY_FEATURE_LORA_TX:
        case FACTORY_FEATURE_LORA_RX:
        case FACTORY_FEATURE_LORA_SETTINGS: fault = snapshot.loraFault; break;
        case FACTORY_FEATURE_IR_TX:
        case FACTORY_FEATURE_IR_RX: fault = snapshot.irFault; break;
        default: break;
        }
        setStatusLed(pageStatusLed_[page], ready, fault);
        if (feature == FACTORY_FEATURE_IR_RX && pageStatusLed_[page] && ready && !fault &&
            snapshot.shieldIrMatch) {
            lv_obj_set_style_bg_color(pageStatusLed_[page], kPanelPressed, 0);
            lv_obj_set_style_border_color(pageStatusLed_[page], kGreen, 0);
        }
    }
}

void FactoryUi::showTestMessage(const char *message)
{
    const char *text = message ? message : "";
    if (activePage_ == kHomePage || activePage_ > featureCount_) return;
    const FactoryFeature feature = pageFeatures_[activePage_];
    if (featureMessage_[feature]) lv_label_set_text(featureMessage_[feature], text);
}

void FactoryUi::showPowerOffDialog(bool usbConnected)
{
    if (powerOffDialog_) return;

    powerOffDialog_ = lv_obj_create(lv_layer_top());
    makeStatic(powerOffDialog_);
    lv_obj_set_size(powerOffDialog_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(powerOffDialog_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(powerOffDialog_, LV_OPA_70, 0);
    lv_obj_set_style_border_width(powerOffDialog_, 0, 0);
    lv_obj_set_style_radius(powerOffDialog_, 0, 0);
    lv_obj_set_style_pad_all(powerOffDialog_, 0, 0);
    lv_obj_set_clickable(powerOffDialog_, true);
    lv_obj_set_user_data(powerOffDialog_, (void *)(uintptr_t)0);
    lv_obj_add_event_cb(powerOffDialog_, powerOffDialogEvent, LV_EVENT_CLICKED, this);

    lv_obj_t *panel = lv_obj_create(powerOffDialog_);
    makeStatic(panel);
    lv_obj_set_width(panel, lv_pct(92));
    lv_obj_set_height(panel, usbConnected ? 154 : 202);
    lv_obj_center(panel);
    stylePanel(panel);
    lv_obj_set_style_pad_left(panel, 12, 0);
    lv_obj_set_style_pad_right(panel, 12, 0);
    lv_obj_set_style_pad_top(panel, 10, 0);
    lv_obj_set_style_pad_bottom(panel, 10, 0);
    lv_obj_set_style_pad_row(panel, 7, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = makeLabel(panel, usbConnected ? "Power off unavailable" : "Enter shipping mode?", 20);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(title, usbConnected ? kRed : kAmber, 0);

    lv_obj_t *message = makeLabel(panel,
                                  usbConnected
                                      ? "USB is connected.\nDisconnect USB before entering shipping mode."
                                      : "After shipping mode, the device can only be activated by reconnecting USB.\nContinue?",
                                  16);
    lv_obj_set_width(message, lv_pct(100));
    lv_obj_set_height(message, usbConnected ? 52 : 76);
    lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t *buttons = lv_obj_create(panel);
    makeStatic(buttons);
    lv_obj_set_width(buttons, lv_pct(100));
    lv_obj_set_height(buttons, 40);
    lv_obj_set_style_bg_opa(buttons, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(buttons, 0, 0);
    lv_obj_set_style_pad_all(buttons, 0, 0);
    lv_obj_set_style_pad_column(buttons, 8, 0);
    lv_obj_set_flex_flow(buttons, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(buttons, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    auto createDialogButton = [this, buttons](const char *text, bool confirm) {
        lv_obj_t *button = lv_button_create(buttons);
        makeStatic(button);
        lv_obj_set_width(button, lv_pct(48));
        lv_obj_set_height(button, 40);
        lv_obj_set_style_bg_color(button, confirm ? kRed : kPanelPressed, 0);
        lv_obj_set_style_border_width(button, 0, 0);
        lv_obj_set_style_radius(button, 7, 0);
        lv_obj_set_user_data(button, (void *)(uintptr_t)(confirm ? 1 : 0));
        lv_obj_add_event_cb(button, powerOffDialogEvent, LV_EVENT_CLICKED, this);
        lv_obj_t *label = makeLabel(button, text, 16);
        lv_obj_set_width(label, lv_pct(100));
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label, confirm ? kBackground : kText, 0);
        lv_obj_center(label);
    };

    if (usbConnected) {
        createDialogButton("OK", false);
    } else {
        createDialogButton("CANCEL", false);
        createDialogButton("ENTER", true);
    }
}

void FactoryUi::actionEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui) return;
    lv_obj_t *button = (lv_obj_t *)lv_event_get_target(event);
    const uint32_t action = (uint32_t)(uintptr_t)lv_obj_get_user_data(button);
    if (action == kActionShieldBase + 3 || action == kActionShieldBase + 5) {
        lv_anim_t animation;
        lv_anim_init(&animation);
        lv_anim_set_var(&animation, button);
        lv_anim_set_exec_cb(&animation, pulseScaleEvent);
        lv_anim_set_values(&animation, 256, 286);
        lv_anim_set_duration(&animation, 100);
        lv_anim_set_playback_duration(&animation, 130);
        lv_anim_start(&animation);
    }
    if (action == kActionDisplay && ui->actions_.displayTest) ui->actions_.displayTest(ui->actions_.context);
    else if (action == kActionSd && ui->actions_.mountSd) ui->actions_.mountSd(ui->actions_.context);
    else if (action == kActionRgb && ui->actions_.pixelTest) ui->actions_.pixelTest(ui->actions_.context);
    else if (action == kActionRotation && ui->actions_.rotateDisplay) ui->actions_.rotateDisplay(ui->actions_.context);
    else if (action == kActionPowerOff && ui->actions_.powerOff) ui->actions_.powerOff(ui->actions_.context);
    else if (action == kActionMagCalibrate && ui->actions_.shieldMagCalibrate) {
        ui->actions_.shieldMagCalibrate(ui->actions_.context);
    } else if (action >= kActionWifiNetworkBase && action < kActionWifiNetworkBase + FACTORY_MAX_WIFI_NETWORKS &&
               ui->actions_.wifiConnect) {
        ui->actions_.wifiConnect(ui->actions_.context, (uint8_t)(action - kActionWifiNetworkBase));
    } else if (action == kActionWifiPing && ui->actions_.wifiPing) {
        ui->actions_.wifiPing(ui->actions_.context);
    } else if (action >= kActionSdMusicTrackBase &&
               action < kActionSdMusicTrackBase + FACTORY_MAX_SD_MUSIC_TRACKS &&
               ui->actions_.sdMusicPlay) {
        ui->actions_.sdMusicPlay(ui->actions_.context,
                                 (uint8_t)(action - kActionSdMusicTrackBase));
    }
    else if (action >= kActionShieldBase + 1 && action <= kActionShieldBase + 17 && ui->actions_.shieldTest) {
        ui->actions_.shieldTest(ui->actions_.context, (uint8_t)(action - kActionShieldBase));
    } else if (action >= kActionLoraConfigBase + 1 && action <= kActionLoraConfigBase + 5 &&
               ui->actions_.shieldConfig) {
        lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(event);
        ui->actions_.shieldConfig(ui->actions_.context, (uint8_t)(action - kActionLoraConfigBase),
                                  (int16_t)lv_slider_get_value(slider));
    } else if (action >= kActionLoraConfigBase + 6 && action <= kActionLoraConfigBase + 9 &&
               ui->actions_.shieldConfig) {
        lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(event);
        ui->actions_.shieldConfig(ui->actions_.context, (uint8_t)(action - kActionLoraConfigBase),
                                  (int16_t)lv_slider_get_value(slider));
    }
}

void FactoryUi::powerOffDialogEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui) return;
    lv_obj_t *target = (lv_obj_t *)lv_event_get_target(event);
    const bool confirmed = (uintptr_t)lv_obj_get_user_data(target) != 0;
    if (ui->powerOffDialog_) {
        lv_obj_del(ui->powerOffDialog_);
        ui->powerOffDialog_ = nullptr;
    }
    if (confirmed && ui->actions_.powerOffConfirmed) {
        ui->actions_.powerOffConfirmed(ui->actions_.context);
    }
}

void FactoryUi::pulseScaleEvent(void *object, int32_t value)
{
    lv_obj_t *button = static_cast<lv_obj_t *>(object);
    if (!button) return;
    lv_obj_set_style_transform_scale(button, value, 0);
}

void FactoryUi::navigateEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui || !ui->tileview_) return;
    lv_obj_t *button = (lv_obj_t *)lv_event_get_target(event);
    const uint8_t page = (uint8_t)(uintptr_t)lv_obj_get_user_data(button);
    lv_obj_set_tile_id(ui->tileview_, page, 0, LV_ANIM_OFF);
    ui->activatePage(page);
}

void FactoryUi::tileChangedEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui) return;
    lv_obj_t *tileview = (lv_obj_t *)lv_event_get_target(event);
    lv_obj_t *tile = lv_tileview_get_tile_active(tileview);
    if (!tile) return;
    ui->activatePage((uint8_t)(uintptr_t)lv_obj_get_user_data(tile));
}

void FactoryUi::activatePage(uint8_t page)
{
    if (page == activePage_) return;
    if (actions_.shieldStop) actions_.shieldStop(actions_.context);
    if (actions_.bleSetActive) actions_.bleSetActive(actions_.context, false);
    activePage_ = page;
    if (page > featureCount_) return;
    if (page > 0 && page <= featureCount_) {
        if (pageFeatures_[page] == FACTORY_FEATURE_WIFI && actions_.wifiScan) {
            actions_.wifiScan(actions_.context);
        } else if (pageFeatures_[page] == FACTORY_FEATURE_SD_MUSIC && actions_.sdMusicScan) {
            actions_.sdMusicScan(actions_.context);
        } else if (pageFeatures_[page] == FACTORY_FEATURE_BLE && actions_.bleSetActive) {
            actions_.bleSetActive(actions_.context, true);
        }
    }
    if (!actions_.shieldTest) return;
    switch (pageFeatures_[page]) {
    case FACTORY_FEATURE_MIC:
        actions_.shieldTest(actions_.context, 2);
        break;
    case FACTORY_FEATURE_LORA_RX:
        actions_.shieldTest(actions_.context, 4);
        break;
    case FACTORY_FEATURE_IR_RX:
        actions_.shieldTest(actions_.context, 6);
        break;
    default:
        break;
    }
}

void FactoryUi::brightnessEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui || !ui->actions_.setBrightness) return;
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(event);
    ui->actions_.setBrightness(ui->actions_.context, (uint8_t)lv_slider_get_value(slider));
}

void FactoryUi::audioVolumeEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui || !ui->actions_.shieldAudioVolume) return;
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(event);
    ui->actions_.shieldAudioVolume(ui->actions_.context, (uint8_t)lv_slider_get_value(slider));
}

void FactoryUi::micSensitivityEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui) return;
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(event);
    ui->micSensitivity_ = lv_slider_get_value(slider);
    if (ui->micSensitivityLabel_) {
        char text[32];
        snprintf(text, sizeof(text), "SENSITIVITY %ldX", (long)ui->micSensitivity_);
        lv_label_set_text(ui->micSensitivityLabel_, text);
    }
}

void FactoryUi::colorTestCloseEvent(lv_event_t *event)
{
    FactoryUi *ui = static_cast<FactoryUi *>(lv_event_get_user_data(event));
    if (!ui || !ui->colorTestOverlay_) return;
    lv_obj_del(ui->colorTestOverlay_);
    ui->colorTestOverlay_ = nullptr;
}

void FactoryUi::showColorTest(lv_color_t color)
{
    if (!colorTestOverlay_) {
        colorTestOverlay_ = lv_obj_create(lv_layer_top());
        lv_obj_set_size(colorTestOverlay_, lv_pct(100), lv_pct(100));
        lv_obj_set_pos(colorTestOverlay_, 0, 0);
        lv_obj_set_style_border_width(colorTestOverlay_, 0, 0);
        lv_obj_set_style_radius(colorTestOverlay_, 0, 0);
        lv_obj_set_style_pad_all(colorTestOverlay_, 0, 0);
        lv_obj_set_style_bg_opa(colorTestOverlay_, LV_OPA_COVER, 0);
        lv_obj_set_clickable(colorTestOverlay_, true);
        lv_obj_add_event_cb(colorTestOverlay_, actionEvent, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(colorTestOverlay_, (void *)(uintptr_t)kActionDisplay);

        lv_obj_t *close = lv_button_create(colorTestOverlay_);
        makeStatic(close);
        lv_obj_set_size(close, 38, 30);
        lv_obj_align(close, LV_ALIGN_TOP_LEFT, 8, 8);
        lv_obj_set_style_bg_color(close, lv_color_make(20, 25, 34), 0);
        lv_obj_set_style_bg_opa(close, LV_OPA_70, 0);
        lv_obj_set_style_border_width(close, 0, 0);
        lv_obj_set_style_radius(close, 5, 0);
        lv_obj_add_event_cb(close, colorTestCloseEvent, LV_EVENT_CLICKED, this);
        lv_obj_t *closeIcon = lv_label_create(close);
        lv_label_set_text(closeIcon, LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(closeIcon, kText, 0);
        lv_obj_center(closeIcon);
    }
    lv_obj_set_style_bg_color(colorTestOverlay_, color, 0);
    lv_obj_move_foreground(colorTestOverlay_);
}
