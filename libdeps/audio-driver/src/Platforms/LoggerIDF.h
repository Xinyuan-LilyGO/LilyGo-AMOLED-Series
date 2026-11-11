#pragma once
#ifdef ESP32
#include "esp_log.h"

#define LOG_METHOD __PRETTY_FUNCTION__
#define TAG_AUDIO_DRIVER "audio-driver"

#define AD_LOGD(...) ESP_LOGD(TAG_AUDIO_DRIVER, __VA_ARGS__)
#define AD_LOGI(...) ESP_LOGI(TAG_AUDIO_DRIVER, __VA_ARGS__)
#define AD_LOGW(...) ESP_LOGW(TAG_AUDIO_DRIVER, __VA_ARGS__)
#define AD_LOGE(...) ESP_LOGE(TAG_AUDIO_DRIVER, __VA_ARGS__)

#define AD_TRACED() ESP_LOGD(TAG_AUDIO_DRIVER, "%s", LOG_METHOD)
#endif
