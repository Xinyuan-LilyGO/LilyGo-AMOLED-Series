/**
 * @file      QWIIC_HP303BSensor.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 * @note      Use Qwiic to connect wemos hp303b temperature and pressure sensor,
 *            the sensor is not included in T-Display-AMOLED, the sketch only demonstrates how to use Wire\
 * !!         Note that the wemos hp303b module is inconsistent with the standard Qwiic line sequence, and jumpers are required
 * !!         See ConnectionDiagram.jpg
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include "src/LOLIN_HP303B.h"       //https://github.com/wemos/LOLIN_HP303B_Library


lv_obj_t *label;
LOLIN_HP303B HP303BPressureSensor;
LilyGo_Class amoled;

// Default HP303B Address 0x77 , may be is 0x76
const uint8_t slaveAddress = 0x77;

void timer_event_handler(lv_timer_t *t)
{
    int32_t temperature;
    int32_t pressure;
    int16_t oversampling = 7;
    int16_t ret, ret1;

    Serial.println();
    //lets the HP303B perform a Single temperature measurement with the last (or standard) configuration
    //The result will be written to the paramerter temperature
    //ret = HP303BPressureSensor.measureTempOnce(temperature);
    //the commented line below does exactly the same as the one above, but you can also config the precision
    //oversampling can be a value from 0 to 7
    //the HP303B will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
    //measurements with higher precision take more time, consult datasheet for more information
    ret = HP303BPressureSensor.measureTempOnce(temperature, oversampling);

    if (ret != 0) {
        //Something went wrong.
        //Look at the library code for more information about return codes
        Serial.print("FAIL! ret = ");
        Serial.println(ret);
    } else {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.println(" degrees of Celsius");
    }

    //Pressure measurement behaves like temperature measurement
    //ret = HP303BPressureSensor.measurePressureOnce(pressure);
    ret1 = HP303BPressureSensor.measurePressureOnce(pressure, oversampling);
    if (ret1 != 0) {
        //Something went wrong.
        //Look at the library code for more information about return codes
        Serial.print("FAIL! ret = ");
        Serial.println(ret1);
    } else {
        Serial.print("Pressure: ");
        Serial.print(pressure);
        Serial.println(" Pascal");
    }

    if (ret1 != 0 || ret != 0) {
        lv_label_set_text(label, "Sensor is not online .");
    } else {
        lv_label_set_text_fmt(label, "Temperature:%d °C\nPressure:%d Pascal", temperature, pressure);
    }
    lv_obj_center(label);
}

void setup(void)
{
    Serial.begin(115200);

    bool rslt = false;

    // Begin LilyGo  1.47 Inch AMOLED board class
    //rslt = amoled.beginAMOLED_147();


    // Begin LilyGo  1.91 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_191();

    // Begin LilyGo  2.41 Inch AMOLED board class
    //rslt =  amoled.beginAMOLED_241();

    // Automatically determine the access device
    rslt = amoled.begin();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }

    // Register lvgl helper
    beginLvglHelper(amoled);

    HP303BPressureSensor.begin(slaveAddress);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);

    label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_timer_create(timer_event_handler, 1000, NULL);
}

void loop()
{
    lv_task_handler();
    delay(5);

}

