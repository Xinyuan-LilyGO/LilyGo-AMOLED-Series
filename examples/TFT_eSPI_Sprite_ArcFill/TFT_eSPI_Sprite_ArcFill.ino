/**
 * @file      TFT_eSPI_Sprite_ArcFill.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-01-31
 * @note      1. Example from https://github.com/Bodmer/TFT_eSPI/tree/master/examples/320%20x%20240/TFT_ArcFill
 *            2. Calling fillScreen will not refresh the full screen.
 *            If you want to use fillScreen, you need to change the default
 *            resolution of TFT_eSPI. Do not change it here. Use fillRect.
 *            3. Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
 *
 * */
#include "esp_arduino_version.h"
#if ESP_ARDUINO_VERSION < ESP_ARDUINO_VERSION_VAL(3,0,0)
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuffers = TFT_eSprite(&tft);
LilyGo_Class amoled;

#define DEG2RAD 0.0174532925

#define LOOP_DELAY 10 // Loop delay to slow things down

uint8_t inc = 0;
unsigned int col = 0;

uint8_t red = 31; // Red is the top 5 bits of a 16 bit colour value
uint8_t green = 0;// Green is the middle 6 bits
uint8_t blue = 0; // Blue is the bottom 5 bits
uint8_t state = 0;

void setup(void)
{
    Serial.begin(115200);

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
    uint16_t width = amoled.width();
    uint16_t height = amoled.height();

    // Continuous segmented (inc*2) elliptical arc drawing
    fillArc(width / 2,  height / 2, ((inc * 2) % 60) * 6, 1, 120, 80, 30, rainbow(col));

    // Circle drawing using arc with arc width = radius
    fillArc(width / 2,  height / 2, inc * 6, 1, 42, 42, 42, rainbow(col));

    inc++;
    col += 1;
    if (col > 191) col = 0;
    if (inc > 59) inc = 0;

    delay(LOOP_DELAY);
}


// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 6 degree segments to draw (60 => 360 degree arc)
// rx = x axis outer radius
// ry = y axis outer radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{

    uint8_t seg = 6; // Segments are 3 degrees wide = 120 segments for 360 degrees
    uint8_t inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

    // Draw colour blocks every inc degrees
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

        // Calculate pair of coordinates for segment end
        float sx2 = cos((i + seg - 90) * DEG2RAD);
        float sy2 = sin((i + seg - 90) * DEG2RAD);
        int x2 = sx2 * (rx - w) + x;
        int y2 = sy2 * (ry - w) + y;
        int x3 = sx2 * rx + x;
        int y3 = sy2 * ry + y;

        framebuffers.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
        framebuffers.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

        // Push framebuffers to amoled
        amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());

        // Copy segment end to sgement start for next segment
        x0 = x2;
        y0 = y2;
        x1 = x3;
        y1 = y3;
    }
}

// #########################################################################
// Return the 16 bit colour with brightness 0-100%
// #########################################################################
unsigned int brightness(unsigned int colour, int brightness)
{
    uint8_t red   = colour >> 11;
    uint8_t green = (colour & 0x7E0) >> 5;
    uint8_t blue  = colour & 0x1F;

    blue =  (blue * brightness) / 100;
    green = (green * brightness) / 100;
    red =   (red * brightness) / 100;

    return (red << 11) + (green << 5) + blue;
}

// #########################################################################
// Return a 16 bit rainbow colour
// #########################################################################
unsigned int rainbow(uint8_t value)
{
    // Value is expected to be in range 0-127
    // The value is converted to a spectrum colour from 0 = blue through to 127 = red

    switch (state) {
    case 0:
        green ++;
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
        green --;
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
    return red << 11 | green << 5 | blue;
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

