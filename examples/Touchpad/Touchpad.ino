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

lv_obj_t *label;

void setup(void)
{
    Serial.begin(115200);

    // Begin LilyGo  1.47 Inch AMOLED board class
    // amoled.beginAMOLED_147();


    // Begin LilyGo  1.91 Inch AMOLED board class
    // amoled.beginAMOLED_191();

    // Automatically determine the access device
    amoled.beginAutomatic();

    // Register lvgl helper
    beginLvglHelper();


    label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);

}

void loop()
{
    static int16_t x, y;
    if (amoled.isPressed()) {
        bool touched = amoled.getPoint(&x, &y);
        if ( touched ) {
            Serial.printf("X:%d Y:%d\n", x, y);
            lv_label_set_text_fmt(label, "X:%d Y:%d", x, y);
            lv_obj_center(label);
        }
    }
    lv_task_handler();
    delay(5);
}

