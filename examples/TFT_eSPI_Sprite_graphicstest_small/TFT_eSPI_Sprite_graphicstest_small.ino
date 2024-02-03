/**
 * @file      TFT_eSPI_Sprite_graphicstest_small.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-01-31
 * @note      1. Example from https://github.com/Bodmer/TFT_eSPI/tree/master/examples/160%20x%20128/TFT_graphicstest_small
 *            2. Calling fillScreen will not refresh the full screen.
 *            If you want to use fillScreen, you need to change the default
 *            resolution of TFT_eSPI. Do not change it here. Use fillRect.
 *            3. Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use framebuffers.xxxx function
 */
#include "esp_arduino_version.h"
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuffers = TFT_eSprite(&tft);
LilyGo_Class amoled;


float p = 3.1415926;

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

    uint16_t time = millis();
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    time = millis() - time;

    Serial.println(time, DEC);
    delay(500);

    // large block of text
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    testdrawtext("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur adipiscing ante sed nibh tincidunt feugiat. Maecenas enim massa, fringilla sed malesuada et, malesuada sit amet turpis. Sed porttitor neque ut ante pretium vitae malesuada nunc bibendum. Nullam aliquet ultrices massa eu hendrerit. Ut sed nisi lorem. In vestibulum purus a tortor imperdiet posuere. ", TFT_WHITE);
    delay(1000);

    // tft print function!
    tftPrintTest();
    delay(4000);

    // a single pixel
    framebuffers.drawPixel(framebuffers.width() / 2, framebuffers.height() / 2, TFT_GREEN);
    delay(500);

    // line draw test
    testlines(TFT_YELLOW);
    delay(500);

    // optimized lines
    testfastlines(TFT_RED, TFT_BLUE);
    delay(500);

    testdrawrects(TFT_GREEN);
    delay(500);

    testfillrects(TFT_YELLOW, TFT_MAGENTA);
    delay(500);

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    testfillcircles(10, TFT_BLUE);
    testdrawcircles(10, TFT_WHITE);
    delay(500);

    testroundrects();
    delay(500);

    testtriangles();
    delay(500);

    mediabuttons();
    delay(500);

    Serial.println("done");
    delay(1000);
}

void loop()
{
    delay(500);
}

void testlines(uint16_t color)
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = 0; x < framebuffers.width(); x += 6) {
        framebuffers.drawLine(0, 0, x, framebuffers.height() - 1, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
    for (int16_t y = 0; y < framebuffers.height(); y += 6) {
        framebuffers.drawLine(0, 0, framebuffers.width() - 1, y, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = 0; x < framebuffers.width(); x += 6) {
        framebuffers.drawLine(framebuffers.width() - 1, 0, x, framebuffers.height() - 1, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
    for (int16_t y = 0; y < framebuffers.height(); y += 6) {
        framebuffers.drawLine(framebuffers.width() - 1, 0, 0, y, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = 0; x < framebuffers.width(); x += 6) {
        framebuffers.drawLine(0, framebuffers.height() - 1, x, 0, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
    for (int16_t y = 0; y < framebuffers.height(); y += 6) {
        framebuffers.drawLine(0, framebuffers.height() - 1, framebuffers.width() - 1, y, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }

    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = 0; x < framebuffers.width(); x += 6) {
        framebuffers.drawLine(framebuffers.width() - 1, framebuffers.height() - 1, x, 0, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
    for (int16_t y = 0; y < framebuffers.height(); y += 6) {
        framebuffers.drawLine(framebuffers.width() - 1, framebuffers.height() - 1, 0, y, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
}

void testdrawtext(char *text, uint16_t color)
{
    framebuffers.setCursor(0, 0);
    framebuffers.setTextColor(color);
    framebuffers.setTextWrap(true);
    framebuffers.print(text);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

}

void testfastlines(uint16_t color1, uint16_t color2)
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t y = 0; y < framebuffers.height(); y += 5) {
        framebuffers.drawFastHLine(0, y, framebuffers.width(), color1);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
    for (int16_t x = 0; x < framebuffers.width(); x += 5) {
        framebuffers.drawFastVLine(x, 0, framebuffers.height(), color2);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
}

void testdrawrects(uint16_t color)
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = 0; x < framebuffers.width(); x += 6) {
        framebuffers.drawRect(framebuffers.width() / 2 - x / 2, framebuffers.height() / 2 - x / 2, x, x, color);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
}

void testfillrects(uint16_t color1, uint16_t color2)
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    for (int16_t x = framebuffers.width() - 1; x > 6; x -= 6) {
        framebuffers.fillRect(framebuffers.width() / 2 - x / 2, framebuffers.height() / 2 - x / 2, x, x, color1);
        framebuffers.drawRect(framebuffers.width() / 2 - x / 2, framebuffers.height() / 2 - x / 2, x, x, color2);
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
}

void testfillcircles(uint8_t radius, uint16_t color)
{
    for (int16_t x = radius; x < framebuffers.width(); x += radius * 2) {
        for (int16_t y = radius; y < framebuffers.height(); y += radius * 2) {
            framebuffers.fillCircle(x, y, radius, color);
            // Push framebuffers to amoled
            amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

        }
    }
}

void testdrawcircles(uint8_t radius, uint16_t color)
{
    for (int16_t x = 0; x < framebuffers.width() + radius; x += radius * 2) {
        for (int16_t y = 0; y < framebuffers.height() + radius; y += radius * 2) {
            framebuffers.drawCircle(x, y, radius, color);
            // Push framebuffers to amoled
            amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

        }
    }
}

void testtriangles()
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    int color = 0xF800;
    int t;
    int w = framebuffers.width() / 2;
    int x = framebuffers.height() - 1;
    int y = 0;
    int z = framebuffers.width();
    for (t = 0 ; t <= 15; t += 1) {
        framebuffers.drawTriangle(w, y, y, x, z, x, color);
        x -= 4;
        y += 4;
        z -= 4;
        color += 100;
        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    }
}

void testroundrects()
{
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    int color = 100;
    int i;
    int t;
    for (t = 0 ; t <= 4; t += 1) {
        int x = 0;
        int y = 0;
        int w = framebuffers.width() - 2;
        int h = framebuffers.height() - 2;
        for (i = 0 ; i <= 16; i += 1) {
            framebuffers.drawRoundRect(x, y, w, h, 5, color);
            x += 2;
            y += 3;
            w -= 4;
            h -= 6;
            color += 1100;
            // Push framebuffers to amoled
            amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

        }
        color += 100;
    }
}

void tftPrintTest()
{
    framebuffers.setTextWrap(false);
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    framebuffers.setCursor(0, 30);
    framebuffers.setTextColor(TFT_RED);
    framebuffers.setTextSize(1);
    framebuffers.println("Hello World!");
    framebuffers.setTextColor(TFT_YELLOW);
    framebuffers.setTextSize(2);
    framebuffers.println("Hello World!");
    framebuffers.setTextColor(TFT_GREEN);
    framebuffers.setTextSize(3);
    framebuffers.println("Hello World!");
    framebuffers.setTextColor(TFT_BLUE);
    framebuffers.setTextSize(4);
    framebuffers.print(1234.567);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(1500);
    framebuffers.setCursor(0, 0);
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    framebuffers.setTextColor(TFT_WHITE);
    framebuffers.setTextSize(0);
    framebuffers.println("Hello World!");
    framebuffers.setTextSize(1);
    framebuffers.setTextColor(TFT_GREEN);
    framebuffers.print(p, 6);
    framebuffers.println(" Want pi?");
    framebuffers.println(" ");
    framebuffers.print(8675309, HEX); // print 8,675,309 out in HEX!
    framebuffers.println(" Print HEX!");
    framebuffers.println(" ");
    framebuffers.setTextColor(TFT_WHITE);
    framebuffers.println("Sketch has been");
    framebuffers.println("running for: ");
    framebuffers.setTextColor(TFT_MAGENTA);
    framebuffers.print(millis() / 1000);
    framebuffers.setTextColor(TFT_WHITE);
    framebuffers.print(" seconds.");
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

}

void mediabuttons()
{
    // play
    // framebuffers.fillScreen(TFT_BLACK);
    framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);

    framebuffers.fillRoundRect(25, 10, 78, 60, 8, TFT_WHITE);
    framebuffers.fillTriangle(42, 20, 42, 60, 90, 40, TFT_RED);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(500);
    // pause
    framebuffers.fillRoundRect(25, 90, 78, 60, 8, TFT_WHITE);
    framebuffers.fillRoundRect(39, 98, 20, 45, 5, TFT_GREEN);
    framebuffers.fillRoundRect(69, 98, 20, 45, 5, TFT_GREEN);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(500);
    // play color
    framebuffers.fillTriangle(42, 20, 42, 60, 90, 40, TFT_BLUE);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

    delay(50);
    // pause color
    framebuffers.fillRoundRect(39, 98, 20, 45, 5, TFT_RED);
    framebuffers.fillRoundRect(69, 98, 20, 45, 5, TFT_RED);
    // play color
    framebuffers.fillTriangle(42, 20, 42, 60, 90, 40, TFT_GREEN);
    // Push framebuffers to amoled
    amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

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