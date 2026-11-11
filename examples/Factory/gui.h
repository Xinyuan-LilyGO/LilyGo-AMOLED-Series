/**
 * @file      gui.h
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-14
 *
 */

#pragma once

#include <lvgl.h>
#include <time.h>
#include <Arduino.h>

void factoryGUI(void);

#define COIN_MSG_ID             0x1000
#define WIFI_MSG_ID             0x1001
#define TEMPERATURE_MSG_ID      0x1002
#define WEATHER_MSG_ID          0x1003

// LVGL 9 removed the optional LVGL 8 message bus. Keep the small subset used
// by Factory as a local adapter so application events remain decoupled.
typedef struct { void *payload; } lv_msg_t;
typedef struct { uint32_t id; lv_obj_t *obj; } FactoryMessageSubscription;
lv_event_code_t factoryMessageEventCode();
void lv_msg_subsribe_obj(uint32_t id, lv_obj_t *obj, void *user_data);
void lv_msg_send(uint32_t id, void *payload);
inline lv_msg_t *lv_event_get_msg(lv_event_t *e) { return (lv_msg_t *)lv_event_get_param(e); }
inline void *lv_msg_get_payload(lv_msg_t *msg) { return msg ? msg->payload : nullptr; }
#define LV_EVENT_MSG_RECEIVED (factoryMessageEventCode())



typedef struct __CoinMarketCapApiSubsribe {
    int id;
    const lv_img_dsc_t *src_img;
    const char *name;
} CoinMarketCapApiSubsribe;

typedef struct __CoinMarketCapApiDataStream {
    int id;
    double price;
    double percent_change_1h;
    double percent_change_24h;
    double percent_change_7d;
} CoinMarketCapApiDataStream;

typedef struct  __OpenWeatherMapApi {
    String city;
    String description;
    double temperature;
    double temp_min;
    double temp_max;
    double pressure;
    double humidity;
} OpenWeatherMapApi;


void selectNextItem();
void showCertification(uint32_t delay_ms);
