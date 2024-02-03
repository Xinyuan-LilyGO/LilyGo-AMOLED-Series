/**
 * @file      TFT_eSPI_Sprite_RLE_Font.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-01-31
 * @note      1. Example from https://github.com/Bodmer/TFT_eSPI/tree/master/examples/320%20x%20240/RLE_Font_test
 *            2. Calling fillScreen will not refresh the full screen.
 *            If you want to use fillScreen, you need to change the default
 *            resolution of TFT_eSPI. Do not change it here. Use fillRect.
 *            3. Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
 */
#include "esp_arduino_version.h"
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI


// New background colour
#define TFT_BROWN 0x38E0

// Pause in milliseconds between screens, change to 0 to time font rendering
#define WAIT 500

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuffers = TFT_eSprite(&tft);
LilyGo_Class amoled;

unsigned long targetTime = 0; // Used for testing draw times

void setup(void)
{
    bool rslt = false;

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

    framebuffers.createSprite(amoled.width(), amoled.height());

    framebuffers.setSwapBytes(1);
}

void loop()
{
    targetTime = millis();

    // First we test them with a background colour set
    framebuffers.setTextSize(1);
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.setTextColor(TFT_GREEN, TFT_BLACK);

    framebuffers.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
    framebuffers.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    framebuffers.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    framebuffers.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);
    int xpos = 0;
    xpos += framebuffers.drawString("xyz{|}~", 0, 64, 2);
    framebuffers.drawChar(127, xpos, 64, 2);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.setTextColor(TFT_GREEN, TFT_BLACK);

    framebuffers.drawString(" !\"#$%&'()*+,-.", 0, 0, 4);
    framebuffers.drawString("/0123456789:;", 0, 26, 4);
    framebuffers.drawString("<=>?@ABCDE", 0, 52, 4);
    framebuffers.drawString("FGHIJKLMNO", 0, 78, 4);
    framebuffers.drawString("PQRSTUVWX", 0, 104, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.drawString("YZ[\\]^_`abc", 0, 0, 4);
    framebuffers.drawString("defghijklmno", 0, 26, 4);
    framebuffers.drawString("pqrstuvwxyz", 0, 52, 4);
    xpos = 0;
    xpos += framebuffers.drawString("{|}~", 0, 78, 4);
    framebuffers.drawChar(127, xpos, 78, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.setTextColor(TFT_BLUE, TFT_BLACK);

    framebuffers.drawString("012345", 0, 0, 6);
    framebuffers.drawString("6789", 0, 40, 6);
    framebuffers.drawString("apm-:.", 0, 80, 6);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.setTextColor(TFT_RED, TFT_BLACK);

    framebuffers.drawString("0123", 0, 0, 7);
    framebuffers.drawString("4567", 0, 60, 7);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.drawString("890:.", 0, 0, 7);
    framebuffers.drawString("", 0, 60, 7);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
    framebuffers.setTextColor(TFT_YELLOW, TFT_BLACK);

    framebuffers.drawString("01", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.drawString("23", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.drawString("45", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.drawString("67", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.drawString("89", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.drawString("0:.", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.setTextColor(TFT_MAGENTA);
    framebuffers.drawNumber(millis() - targetTime, 0, 100, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(4000);

    // Now test them with transparent background
    targetTime = millis();

    framebuffers.setTextSize(1);
    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);

    framebuffers.setTextColor(TFT_GREEN);

    framebuffers.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
    framebuffers.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    framebuffers.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    framebuffers.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);
    xpos = 0;
    xpos += framebuffers.drawString("xyz{|}~", 0, 64, 2);
    framebuffers.drawChar(127, xpos, 64, 2);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.setTextColor(TFT_GREEN);

    framebuffers.drawString(" !\"#$%&'()*+,-.", 0, 0, 4);
    framebuffers.drawString("/0123456789:;", 0, 26, 4);
    framebuffers.drawString("<=>?@ABCDE", 0, 52, 4);
    framebuffers.drawString("FGHIJKLMNO", 0, 78, 4);
    framebuffers.drawString("PQRSTUVWX", 0, 104, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);
    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.drawString("YZ[\\]^_`abc", 0, 0, 4);
    framebuffers.drawString("defghijklmno", 0, 26, 4);
    framebuffers.drawString("pqrstuvwxyz", 0, 52, 4);
    xpos = 0;
    xpos += framebuffers.drawString("{|}~", 0, 78, 4);
    framebuffers.drawChar(127, xpos, 78, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.setTextColor(TFT_BLUE);

    framebuffers.drawString("012345", 0, 0, 6);
    framebuffers.drawString("6789", 0, 40, 6);
    framebuffers.drawString("apm-:.", 0, 80, 6);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.setTextColor(TFT_RED);

    framebuffers.drawString("0123", 0, 0, 7);
    framebuffers.drawString("4567", 0, 60, 7);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.drawString("890:.", 0, 0, 7);
    framebuffers.drawString("", 0, 60, 7);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.setTextColor(TFT_YELLOW);

    framebuffers.drawString("0123", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.drawString("4567", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    // framebuffers.fillScreen(TFT_BROWN);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BROWN);
    framebuffers.drawString("890:.", 0, 0, 8);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(WAIT);

    framebuffers.setTextColor(TFT_MAGENTA);

    framebuffers.drawNumber(millis() - targetTime, 0, 100, 4);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(4000);;
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