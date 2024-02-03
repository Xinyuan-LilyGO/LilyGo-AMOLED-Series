/**
 * @file      TFT_eSPI_Sprite.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-14
 * @note      Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
 */



#include "esp_arduino_version.h"
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI
#include "true_color.h"
#include <LV_Helper.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
LilyGo_Class amoled;

#define WIDTH  amoled.width()
#define HEIGHT amoled.height()
unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;

void setup()
{
    // Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
    Serial.begin(115200);

    // Begin LilyGo  1.47 Inch AMOLED board class
    // amoled.beginAMOLED_147();


    // Begin LilyGo  1.91 Inch AMOLED board class
    // amoled.beginAMOLED_191();

    // Begin LilyGo  2.41 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_241();

    // Automatically determine the access device
    if (!amoled.begin()) {
        while (1) {
            Serial.println("There is a problem with the device!~"); delay(1000);
        }
    }

    spr.createSprite(WIDTH, HEIGHT);

    spr.setSwapBytes(1);
}

void loop()
{
    // This is a picture for 1.47 inch screen size, other sizes will not fit
    size_t examples_image_size = sizeof(gImage_true_color) / sizeof(gImage_true_color[0]);
    if (examples_image_size == (WIDTH * HEIGHT * sizeof(uint16_t))) {
        spr.pushImage(0, 0, WIDTH, HEIGHT, (uint16_t *)gImage_true_color);
        amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
        delay(2000);
    }

    spr.fillSprite(TFT_BLACK);

    spr.fillRect(0,   0,  67, 120,    TFT_RED);
    spr.fillRect(67 * 1,  0, 67, 120, TFT_GREEN);
    spr.fillRect(67 * 2,  0, 67, 120, TFT_BLUE);
    spr.fillRect(67 * 3,  0, 67, 120, TFT_RED);
    spr.fillRect(67 * 4,  0, 67, 120, TFT_GREEN);
    spr.fillRect(67 * 5,  0, 67, 120, TFT_BLUE);
    spr.fillRect(67 * 6,  0, 67, 120, TFT_RED);
    spr.fillRect(67 * 7,  0, 67, 120, TFT_GREEN);

    spr.fillRect(0,       120, 67, 120, TFT_BLUE);
    spr.fillRect(67 * 1,  120, 67, 120, TFT_RED);
    spr.fillRect(67 * 2,  120, 67, 120, TFT_GREEN);
    spr.fillRect(67 * 3,  120, 67, 120, TFT_BLUE);
    spr.fillRect(67 * 4,  120, 67, 120, TFT_RED);
    spr.fillRect(67 * 5,  120, 67, 120, TFT_GREEN);
    spr.fillRect(67 * 6,  120, 67, 120, TFT_BLUE);
    spr.fillRect(67 * 7,  120, 67, 120, TFT_RED);

    amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
    delay(2000);

    uint16_t colors[6] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
    for (int i = 0; i < 6; ++i) {
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(colors[i], TFT_BLACK);
        spr.drawString("LilyGo.cc", WIDTH / 2 - 30, 85, 4);
        spr.drawString("T-Display AMOLED", WIDTH / 2 - 70, 110, 4);
        amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
        delay(200);
    }
    delay(2000);


    for (int pos = WIDTH; pos > 0; pos--) {
        int h = HEIGHT;
        while (h--) spr.drawFastHLine(0, h, WIDTH, rainbow(h * 4));
        spr.setTextSize(1);
        spr.setTextFont(4);
        spr.setTextColor(TFT_WHITE);
        spr.setTextWrap(false);
        spr.setCursor(pos, 100);
        spr.print("LilyGo AMOLED");
        amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
    }
    delay(2000);


    targetTime = millis() + 1000;
    uint32_t runTime = millis() + 6000;
    spr.fillSprite(TFT_BLACK);
    while (runTime > millis()) {
        drawRainbow();
    }
    delay(2000);

}



void drawRainbow()
{
    if (targetTime < millis()) {
        targetTime = millis() + 500;
        Serial.println(targetTime);
        // Colour changing state machine
        for (int i = 0; i < WIDTH; i++) {
            spr.drawFastVLine(i, 0, spr.height(), colour);
            switch (state) {
            case 0:
                green += 2;
                if (green == 64) {
                    green = 63;
                    state = 1;
                }
                break;
            case 1:
                red--;
                if (red == 255) {
                    red = 0;
                    state = 2;
                }
                break;
            case 2:
                blue ++;
                if (blue == 32) {
                    blue = 31;
                    state = 3;
                }
                break;
            case 3:
                green -= 2;
                if (green == 255) {
                    green = 0;
                    state = 4;
                }
                break;
            case 4:
                red ++;
                if (red == 32) {
                    red = 31;
                    state = 5;
                }
                break;
            case 5:
                blue --;
                if (blue == 255) {
                    blue = 0;
                    state = 0;
                }
                break;
            }
            colour = red << 11 | green << 5 | blue;
        }

        // The standard ADAFruit font still works as before
        spr.setTextColor(TFT_BLACK);
        spr.setCursor (12, 5);
        spr.print("Original ADAfruit font!");

        // The new larger fonts do not use the .setCursor call, coords are embedded
        spr.setTextColor(TFT_BLACK, TFT_BLACK); // Do not plot the background colour

        // Overlay the black text on top of the rainbow plot (the advantage of not drawing the background colour!)
        spr.drawCentreString("Font size 2", 80, 14, 2); // Draw text centre at position 80, 12 using font 2

        //spr.drawCentreString("Font size 2",81,12,2); // Draw text centre at position 80, 12 using font 2

        spr.drawCentreString("Font size 4", 80, 30, 4); // Draw text centre at position 80, 24 using font 4

        spr.drawCentreString("12.34", 80, 54, 6); // Draw text centre at position 80, 24 using font 6

        spr.drawCentreString("12.34 is in font size 6", 80, 92, 2); // Draw text centre at position 80, 90 using font 2

        // Note the x position is the top left of the font!

        // draw a floating point number
        float pi = 3.14159; // Value to print
        int precision = 3;  // Number of digits after decimal point
        int xpos = 50;      // x position
        int ypos = 110;     // y position
        int font = 2;       // font number only 2,4,6,7 valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 0 : a p m
        xpos += spr.drawFloat(pi, precision, xpos, ypos, font); // Draw rounded number and return new xpos delta for next print position
        spr.drawString(" is pi", xpos, ypos, font); // Continue printing from new x position
        amoled.pushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
    }
}


// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(uint8_t value)
{
    // Value is expected to be in range 0-127
    // The value is converted to a spectrum colour from 0 = red through to 127 = blue

    uint8_t red   = 0; // Red is the top 5 bits of a 16 bit colour value
    uint8_t green = 0;// Green is the middle 6 bits
    uint8_t blue  = 0; // Blue is the bottom 5 bits

    uint8_t sector = value >> 5;
    uint8_t amplit = value & 0x1F;

    switch (sector) {
    case 0:
        red   = 0x1F;
        green = amplit;
        blue  = 0;
        break;
    case 1:
        red   = 0x1F - amplit;
        green = 0x1F;
        blue  = 0;
        break;
    case 2:
        red   = 0;
        green = 0x1F;
        blue  = amplit;
        break;
    case 3:
        red   = 0;
        green = 0x1F - amplit;
        blue  = 0x1F;
        break;
    }

    return red << 11 | green << 6 | blue;
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