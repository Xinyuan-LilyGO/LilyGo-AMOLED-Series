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