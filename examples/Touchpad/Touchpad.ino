/**
 * @file      Touchpad.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-07-14
 *
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;
lv_obj_t *label;

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


    label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Touch test");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_center(label);

    // Only 1.91 Inch AMOLED board support
    amoled.setHomeButtonCallback([](void *ptr) {
        Serial.println("Home key pressed!");
        static uint32_t checkMs = 0;
        if (millis() > checkMs) {
            lv_label_set_text(label, "Home Pressed");
            lv_obj_center(label);
        }
        checkMs = millis() + 200;
    }, NULL);

}

void loop()
{
    static int16_t x, y;
    bool touched = amoled.getPoint(&x, &y);
    if ( touched ) {
        Serial.printf("X:%d Y:%d\n", x, y);
        lv_label_set_text_fmt(label, "X:%d Y:%d", x, y);
        lv_obj_center(label);
    }
    lv_task_handler();
    delay(5);
}

