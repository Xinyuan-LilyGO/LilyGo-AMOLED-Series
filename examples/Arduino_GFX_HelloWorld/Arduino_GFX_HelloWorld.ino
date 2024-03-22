/**
 * @file      Arduino_GFX_HelloWorld.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2024-03-21
 *
 */
#include <Arduino.h>
#include <Arduino_GFX_Library.h> // https://github.com/moononournation/Arduino_GFX.git
#include "TouchDrvCSTXXX.hpp"

#define GFX_DEV_DEVICE LILYGO_T_DISPLAY_S3_AMOLED

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    6 /* cs */, 47 /* sck */, 18 /* d0 */, 7 /* d1 */, 48 /* d2 */, 5 /* d3 */);
Arduino_GFX *gfx = new Arduino_RM67162(bus, 17 /* RST */, 0 /* rotation */);
Arduino_GFX *gfx2;
TouchDrvCSTXXX touch;
bool _touchOnline = false;

void setBrightness(uint8_t value)
{
    bus->beginWrite();
    bus->writeC8D8(0x51, value);
    bus->endWrite();
}


void setup()
{
    Serial.begin(115200);

    /**
     * * The difference between the touch and non-touch versions is that the display
     * * power supply of the touch version is controlled by IO38
    */
    pinMode(38, OUTPUT);
    digitalWrite(38, OUTPUT);

    touch.setPins(-1, 21);
    bool res = touch.begin(Wire, CST816_SLAVE_ADDRESS, 3, 2);
    if (!res) {
        Serial.println("Failed to find CST816T - check your wiring!");
        _touchOnline = false;
    } else {
        _touchOnline = true;
        touch.setCenterButtonCoordinate(600, 120);  //AMOLED 1.91 inch
        Serial.println("CST816T init success!");
        // Only 1.91 Inch AMOLED board support
        touch.setHomeButtonCallback([](void *ptr) {
            Serial.println("Home key pressed!");
        }, NULL);
    }

    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }

    gfx2 = new Arduino_Canvas(240, 536, gfx, 0, 0); // for Sprites
    gfx2->begin(GFX_SKIP_OUTPUT_BEGIN); // Added the GFX_SKIP_OUTPUT_BEGIN so the Canvas class doesn’t try and initialise the display
    gfx2->fillScreen(BLACK);
    gfx2->setCursor(80, 268);
    gfx2->setTextColor(RED);
    gfx2->setTextSize(2 /* x scale */, 2 /* y scale */, 1 /* pixel_margin */);
    gfx2->println("Hello World!");
    gfx2->fillCircle(130, 130, 40, GREEN);
    gfx2->flush();

    //Test brightness
    for (int i = 0; i < 255; i++) {
        setBrightness(i);
        delay(20);
    }

}

void loop()
{
    if (_touchOnline) {
        if (touch.isPressed()) {
            int16_t x_array[1]; int16_t y_array[1];
            if (touch.getPoint(x_array, y_array)) {
                Serial.printf("X:%d Y:%d \n", x_array[0], y_array[0]);
            }
        }
    }
    delay(5);
}