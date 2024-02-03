/**
 * @file      TFT_eSPI_Sprite_Rotation.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-14
 * @note      Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
 * @note      Screen rotation only supports 2.41 Inch or 1.91-inch touch and non-touch versions
 * @note      Screen rotation only supports 2.41 Inch or 1.91-inch touch and non-touch versions
 */
#include "esp_arduino_version.h"
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI


TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
LilyGo_Class amoled;

uint8_t rotation = 0;



void setup()
{
    bool rslt = false;

    // Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
    Serial.begin(115200);

    // 1.47 inches does not support rotation, this sketch cannot be used
    //rslt = amoled.beginAMOLED_147();

    // Begin LilyGo  1.91 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_191();

    // Begin LilyGo  2.41 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_241();

    // Automatically determine the access device
    rslt = amoled.begin();

    if (!rslt) {
        while (1) {
            Serial.println("There is a problem with the device!~"); delay(1000);
        }
    }

    spr.createSprite(amoled.width(), amoled.height());

    spr.setSwapBytes(1);
}

void loop()
{
    static int16_t x, y;


    uint16_t colors[6] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
    for (int i = 0; i < 6; ++i) {
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(colors[i], TFT_BLACK);
        spr.drawString("T-Display AMOLED", amoled.width() / 2 - 70, 110, 4);
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)spr.getPointer());
        delay(200);
    }
    delay(2000);

    amoled.setRotation(rotation++);
    rotation %= 4;
    spr.deleteSprite();
    spr.createSprite( amoled.width(), amoled.height());
    spr.setSwapBytes(1);
}


#else

#include <Arduino.h>

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    Serial.println("The current arduino version of TFT_eSPI does not support arduino 3.0, please change the version to below 3.0");
    delay(1000);
}

#endif