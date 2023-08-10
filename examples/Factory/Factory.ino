/**
 * @file      Factory.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-13
 * @note      Arduino Setting
 *            Tools ->
 *                  Board:"ESP32S3 Dev Module"
 *                  USB CDC On Boot:"Enable"
 *                  USB DFU On Boot:"Disable"
 *                  Flash Size : "16MB(128Mb)"
 *                  Flash Mode"QIO 80MHz
 *                  Partition Scheme:"16M Flash(3M APP/9.9MB FATFS)"
 *                  PSRAM:"OPI PSRAM"
 *                  Upload Mode:"UART0/Hardware CDC"
 *                  USB Mode:"Hardware CDC and JTAG"
 *  Arduino IDE User need move all folders in [libdeps folder](./libdeps/)  to Arduino library folder
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <time.h>
#include <lvgl.h>
#include "gui.h"
#include "sntp.h"
#include "zones.h"
#include <Adafruit_NeoPixel.h>      //https://github.com/adafruit/Adafruit_NeoPixel
#include <AceButton.h>
#include <cJSON.h>
#include "rootCa.h"
#include <esp_wifi.h>

using namespace ace_button;

//! You can use EspTouch to configure the network key without changing the WiFi password below
#ifndef WIFI_SSID
#define WIFI_SSID             "Your WiFi SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD         "Your WiFi PASSWORD"
#endif

// !Your Coinmarketcap API KEY , See https://coinmarketcap.com/api/
#ifndef COINMARKETCAP_APIKEY
#define COINMARKETCAP_APIKEY   ""
#endif
// !Your OpenWeatherMap API KEY, See https://openweathermap.org/api
#ifndef OPENWEATHERMAP_APIKEY
#define OPENWEATHERMAP_APIKEY   ""
#endif

// !Geographical coordinates (latitude) , See https://openweathermap.org/current
#ifndef OPENWEATHERMAP_LAT
#define OPENWEATHERMAP_LAT      "22.64610787469689"
#endif

// !Geographical coordinates (longitude) , See https://openweathermap.org/current
#ifndef OPENWEATHERMAP_LON
#define OPENWEATHERMAP_LON      "114.05498017285154"
#endif

// For temperature in Celsius
#define OPENWEATHERMAP_USE_CELSIUS

// For temperature in Fahrenheit
// #define OPENWEATHERMAP_USE_FAHRENHEIT


#define NTP_SERVER1           "pool.ntp.org"
#define NTP_SERVER2           "time.nist.gov"
#define GMT_OFFSET_SEC        0
#define DAY_LIGHT_OFFSET_SEC  0
#define GET_TIMEZONE_API      "https://ipapi.co/timezone/"
#define COINMARKETCAP_HOST    "pro-api.coinmarketcap.com"


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(BOARD_PIXELS_NUM, BOARD_PIXELS_PIN, NEO_GRB + NEO_KHZ800);

void WiFiEvent(WiFiEvent_t event);
void datetimeSyncTask(void *ptr);
void updateCoin360Task(void *ptr);
WiFiClientSecure client ;
HTTPClient https;
AceButton button1;
AceButton button2;

double latitude;
double longitude;
String timezone = "";

static OpenWeatherMapApi weatherApi;
static TaskHandle_t  vUpdateDateTimeTaskHandler = NULL;
static TaskHandle_t  vUpdateWeatherTaskHandler = NULL;
static TaskHandle_t  vUpdateCoin360TaskHandler = NULL;
static SemaphoreHandle_t xWiFiLock = NULL;
static CoinMarketCapApiDataStream coinData[4];
static String httpBody;
extern CoinMarketCapApiSubsribe coinSubsribe[4] ;


void buttonHandleEvent(AceButton *button,
                       uint8_t eventType,
                       uint8_t buttonState)
{
    switch (eventType) {
    case AceButton::kEventClicked:
        // Toggle CHG led
        amoled.setChargingLedMode(
            amoled.getChargingLedMode() != XPOWERS_CHG_LED_OFF ?
            XPOWERS_CHG_LED_OFF : XPOWERS_CHG_LED_ON);
        break;
    default:
        break;
    }
}

void buttonHandlerTask(void *ptr)
{
    while (1) {
        button1.check();
        button2.check();
        delay(5);
    }
    vTaskDelete(NULL);
}


void setup()
{
    Serial.begin(115200);

    xWiFiLock =  xSemaphoreCreateBinary();
    xSemaphoreGive( xWiFiLock );

    // Register WiFi event
    WiFi.onEvent(WiFiEvent);

    pixels.begin(); // This initializes the NeoPixel library.
    pixels.setBrightness(15);

    // Test pixels color
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); pixels.show(); delay(500);
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); pixels.show(); delay(500);
    pixels.setPixelColor(0, pixels.Color(0, 0, 255)); pixels.show(); delay(500);
    pixels.clear();
    pixels.show();

    //Set button on/off charge led , just for test button
    pinMode(BOARD_BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BOARD_BUTTON2_PIN, INPUT_PULLUP);


    ButtonConfig *buttonConfig = button1.getButtonConfig();
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setEventHandler(buttonHandleEvent);

    button2.init(BOARD_BUTTON2_PIN);
    buttonConfig = button2.getButtonConfig();
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setEventHandler(buttonHandleEvent);

    // begin LilyGo amoled board class
    amoled.beginAMOLED_147();

    // Register lvgl helper
    beginLvglHelper();

    // Draw Factory GUI
    factoryGUI();

    wifi_config_t current_conf = {0};

    WiFi.mode(WIFI_STA);

    // Connect to the Internet after initializing the UI.
    esp_wifi_get_config(WIFI_IF_STA, &current_conf);
    if (strlen((const char *)current_conf.sta.ssid) == 0) {
        // Just for factory testing.
        Serial.println("Use default WiFi SSID & PASSWORD!!");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    } else {
        Serial.println("Begin WiFi");
        WiFi.begin();
    }


    // Enable Watchdog
    enableLoopWDT();

    xTaskCreate(buttonHandlerTask, "btn", 5 * 1024, NULL, 12, NULL);

}

void loop()
{
    lv_task_handler();
    delay(1);
}


void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        lv_msg_send(WIFI_MSG_ID, NULL);
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);

        if (!vUpdateDateTimeTaskHandler) {
            xTaskCreate(datetimeSyncTask, "sync", 10 * 1024, NULL, 12, &vUpdateDateTimeTaskHandler);
        }
        if (!vUpdateCoin360TaskHandler) {
            if (String(COINMARKETCAP_APIKEY) != "") {
                xTaskCreate(updateCoin360Task, "coin", 10 * 1024, NULL, 10, &vUpdateCoin360TaskHandler);
            }
        }
        if (!vUpdateWeatherTaskHandler) {
            if (String(OPENWEATHERMAP_APIKEY) != "") {
                xTaskCreate(updateWeatherTask, "weather", 30 * 1024, NULL, 9, &vUpdateWeatherTaskHandler);
            }

        }
        lv_msg_send(WIFI_MSG_ID, NULL);
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        lv_msg_send(WIFI_MSG_ID, NULL);
        break;
    default: break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// DATETIME DATETIME DATETIME DATETIME
/////////////////////////////////////////////////////////////////////////////////////////


void datetimeSyncTask(void *ptr)
{
    int httpCode;
    while (1) {
        delay(5000);

        if ( xSemaphoreTake( xWiFiLock, portMAX_DELAY ) == pdTRUE ) {

            client.setCACert(rootCACertificate);
            if (https.begin(client, GET_TIMEZONE_API)) {
                httpCode = https.GET();
                if (httpCode > 0) {
                    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                        httpBody = https.getString();
                    }
                } else {
                    Serial.printf("[HTTPS] GET... failed, error: %s\n",
                                  https.errorToString(httpCode).c_str());
                }
                https.end();
            }

            client.stop();

            if (httpCode == HTTP_CODE_OK ) {
                for (uint32_t i = 0; i < sizeof(zones); i++) {
                    if (httpBody == "none") {
                        httpBody = "CST-8";
                        break;
                    }
                    if (httpBody == zones[i].name) {
                        httpBody = zones[i].zones;
                        break;
                    }
                }
                Serial.println("timezone : " + httpBody);
                setenv("TZ", httpBody.c_str(), 1); // set time zone
                tzset();
                xSemaphoreGive( xWiFiLock );

                vUpdateDateTimeTaskHandler = NULL;
                // Just run once
                vTaskDelete(NULL);
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////////////////////////
// CoinMarketCapApi CoinMarketCapApi CoinMarketCapApi
/////////////////////////////////////////////////////////////////////////////////////////

// The order of acquisition needs to be consistent with the icon
void updateCoin360Task(void *ptr)
{
    while (1) {

        cJSON *root = NULL;

        bool done = false;

        bool findHeaders = false;

        delay(10000);

        while (!WiFi.isConnected()) {
            delay(1000);
        }

        if ( xSemaphoreTake( xWiFiLock, portMAX_DELAY ) == pdTRUE ) {

            httpBody = "";

            client.setCACert(CoinMarketCapApiRootCA);

            String url = "/v1/cryptocurrency/quotes/latest?symbol=";
            int counter = sizeof(coinSubsribe) / sizeof(coinSubsribe[0]);
            for (int i = 0; i < counter; ++i) {
                url += coinSubsribe[i].name;
                if (i != counter - 1) {
                    url += ",";
                }
            }
            url += "&convert=USD";

            client.connect("pro-api.coinmarketcap.com", 443);
            client.println("GET " + url + " HTTP/1.1");
            client.println("Host: " COINMARKETCAP_HOST);
            client.println("User-Agent: arduino/1.0.0");
            client.println("Accepts: application/json");
            client.print("X-CMC_PRO_API_KEY: ");
            client.println(COINMARKETCAP_APIKEY);
            client.println();
            uint32_t now = millis();
            while (millis() - now < 3000) {
                while (client.available()) {
                    char c = client.read();
                    if (!findHeaders) {
                        if (c == '{') {
                            findHeaders = true;
                            httpBody = c;
                        }
                    } else {
                        if (c == '\r') {
                            break;
                        } else {
                            httpBody += c;
                        }
                    }
                    delay(1);
                }
            }

            client.stop();
            Serial.println(httpBody);

            // Use cJSON parse data stream
            root = cJSON_Parse(httpBody.c_str());
            if (root) {
                cJSON *data = cJSON_GetObjectItem(root, "data");
                if (data) {
                    int size =  cJSON_GetArraySize(data);
                    for (int i = 0; i < size; i++) {
                        cJSON *__coinId = cJSON_GetObjectItem(data, coinSubsribe[i].name);
                        cJSON *id = cJSON_GetObjectItem(__coinId, "id");
                        if (__coinId) {
                            cJSON *quote = cJSON_GetObjectItem(__coinId, "quote");
                            if (quote) {
                                cJSON *__currency = cJSON_GetObjectItem(quote, "USD");
                                if (__currency) {
                                    cJSON *price = cJSON_GetObjectItem(__currency, "price");
                                    cJSON *percent_change_1h = cJSON_GetObjectItem(__currency, "percent_change_1h");
                                    cJSON *percent_change_24h = cJSON_GetObjectItem(__currency, "percent_change_24h");
                                    cJSON *percent_change_7d = cJSON_GetObjectItem(__currency, "percent_change_7d");
                                    Serial.print( coinSubsribe[i].name); Serial.print(":");
                                    Serial.print("\t"); Serial.print("ID:"); Serial.println(id->valueint);
                                    Serial.print("\t"); Serial.print("Price:"); Serial.println(price->valuedouble);
                                    Serial.print("\t"); Serial.print("1H:"); Serial.println(percent_change_1h->valuedouble);
                                    Serial.print("\t"); Serial.print("24H:"); Serial.println(percent_change_24h->valuedouble);
                                    Serial.print("\t"); Serial.print("7D:"); Serial.println(percent_change_7d->valuedouble);
                                    if (id) {
                                        coinData[i].id = id->valueint;
                                    }
                                    coinData[i].price = price->valuedouble;
                                    coinData[i].percent_change_1h = percent_change_1h->valuedouble;
                                    coinData[i].percent_change_24h = percent_change_24h->valuedouble;
                                    coinData[i].percent_change_7d = percent_change_7d->valuedouble;
                                    lv_msg_send(COIN_MSG_ID, &coinData[i]);
                                    done = true;
                                }
                            }
                        }
                    }
                }
            }
            xSemaphoreGive( xWiFiLock );
        }
        if (root) {
            cJSON_free(root);
        }

        delay(done ? 30 * 60000 : 60000);
    }
}





/////////////////////////////////////////////////////////////////////////////////////////
// WeatherApi WeatherApi WeatherApi WeatherApi
/////////////////////////////////////////////////////////////////////////////////////////

void updateWeatherTask(void *ptr)
{
    while (1) {
        int httpCode = -1;
        cJSON *root = NULL;

        bool done = false;

        delay(15000);

        while (!WiFi.isConnected()) {
            delay(1000);
        }

        if ( xSemaphoreTake( xWiFiLock, portMAX_DELAY ) == pdTRUE ) {

            httpBody = "";

            // FreeAPI : https://api.openweathermap.org/data/2.5/weather?lat={lat}&lon={lon}&appid={API key}
            String url = "https://api.openweathermap.org/data/2.5/weather?lat=";
#ifdef OPENWEATHERMAP_LAT
            url += OPENWEATHERMAP_LAT;
#else
            url += String(latitude);
#endif
            url += "&lon=";

#ifdef OPENWEATHERMAP_LON
            url += OPENWEATHERMAP_LON;
#else
            url += String(longitude);
#endif

#if defined(OPENWEATHERMAP_USE_FAHRENHEIT)
            url += "&units=imperial";
#elif  defined(OPENWEATHERMAP_USE_CELSIUS)
            url += "&units=metric";
#endif
            url += "&appid=";
            url += OPENWEATHERMAP_APIKEY;

            client.setCACert(OpenWeatherRootCA);

            if (https.begin(client, url)) {
                httpCode = https.GET();
                if (httpCode > 0) {
                    Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
                    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                        httpBody = https.getString();
                    }
                } else {
                    Serial.printf("[HTTPS] GET... failed, error: %s\n",
                                  https.errorToString(httpCode).c_str());
                }
                https.end();
            }


            client.stop();

            while (1) {

                if (httpCode != HTTP_CODE_OK) {
                    break;
                }

                Serial.println(httpBody.c_str());

                // Use cJSON parse data stream
                root = cJSON_Parse(httpBody.c_str());
                if (!root) {
                    Serial.println("GET ROOT FAILED");
                    break;
                }
                // Get City string
                cJSON *city = cJSON_GetObjectItem(root, "name");
                if (city) {
                    weatherApi.city = city->valuestring;
                }

                // Get weather string
                cJSON *weather = cJSON_GetObjectItem(root, "weather");
                if (!weather) {
                    Serial.println("GET weather FAILED");
                    break;
                }
                cJSON *weatherItem = cJSON_GetArrayItem(weather, 0);
                if (!weatherItem) {
                    Serial.println("GET weatherItem FAILED");
                    break;
                }
                cJSON *description = cJSON_GetObjectItem(weatherItem, "description");
                if (description) {
                    weatherApi.description = description->valuestring;
                }

                // Get main string
                cJSON *main = cJSON_GetObjectItem(root, "main");
                if (!main) {
                    Serial.println("GET main FAILED");
                    break;
                }
                cJSON *temperature = cJSON_GetObjectItem(main, "temp");
                if (temperature) {
                    weatherApi.temperature = temperature->valuedouble;
                }
                cJSON *temp_min = cJSON_GetObjectItem(main, "temp_min");
                if (temp_min) {
                    weatherApi.temp_min = temp_min->valuedouble;
                }
                cJSON *temp_max = cJSON_GetObjectItem(main, "temp_max");
                if (temp_max) {
                    weatherApi.temp_max = temp_max->valuedouble;
                }
                cJSON *pressure = cJSON_GetObjectItem(main, "pressure");
                if (pressure) {
                    weatherApi.pressure = pressure->valuedouble;
                }
                cJSON *humidity = cJSON_GetObjectItem(main, "humidity");
                if (humidity) {
                    weatherApi.humidity = humidity->valuedouble;
                }

                Serial.println("Weather:");
                Serial.print("\tCity:");
                Serial.println(weatherApi.city);

                Serial.print("\tdescription:");
                Serial.println(weatherApi.description);

                Serial.print("\ttemperature:");
                Serial.println(weatherApi.temperature);

                Serial.print("\ttemp_min:");
                Serial.println(weatherApi.temp_min);

                Serial.print("\ttemp_max:");
                Serial.println(weatherApi.temp_max);

                Serial.print("\tpressure:");
                Serial.println(weatherApi.pressure);

                Serial.print("\thumidity:");
                Serial.println(weatherApi.humidity);

                lv_msg_send(WEATHER_MSG_ID, &weatherApi);
                done = true;
                break;
            }

            if (root) {
                cJSON_free(root);
            }
            xSemaphoreGive( xWiFiLock );
        }
        /*
        * https://openweathermap.org/price
        * 60 calls/minute
        * 1,000,000 calls/month
        * */
        delay(done ? 30 * 60000 : 60000);
    }
}
