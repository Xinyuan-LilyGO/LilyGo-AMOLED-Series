/**
 * @file      QWIIC_MAX3010X.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-10-09
 * @note      Connect the external MAX30102 heart rate sensor using the QWIIC interface,
 *            The sensor is not included on the board and requires external connection
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <MAX30105.h>   //https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library


LilyGo_Class amoled;
MAX30105 particleSensor;
lv_obj_t *label;
uint32_t checkInterVal = 0;

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

    // Initialize sensor
    if (!particleSensor.begin(Wire)) {
        while (1) {
            Serial.println(F("MAX3010x was not found. Please check wiring/power."));
            delay(1000);
        }
    }

    particleSensor.setup(); //Configure sensor. Use 6.4mA for LED drive

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);
}

void loop()
{
    if (millis() > checkInterVal) {
        uint32_t irValue = particleSensor.getIR();
        uint32_t redValue = particleSensor.getRed();
        float temperature = particleSensor.readTemperature();
        Serial.print(" R[");
        Serial.print(redValue);
        Serial.print("] IR[");
        Serial.print(irValue);
        Serial.print("] Temp:[");
        Serial.print(temperature);
        Serial.println("]");
        lv_label_set_text_fmt(label, "IR : %u\nRed : %u \nTemperature:%.2f", irValue, redValue, temperature);
        checkInterVal += 100;
    }
    lv_task_handler();
    delay(5);
}

