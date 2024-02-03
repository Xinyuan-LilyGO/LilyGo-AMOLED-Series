/**
 * @file      LumenMeter.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 * @note      Sketch only for 1.47" screen with built-in ambient light sensor
 */

#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;
lv_obj_t *label;


void timer_event_handler(lv_timer_t *t)
{
    float lux =  amoled.getLux();
    lv_label_set_text_fmt(label, "%.2f lux", lux);
    lv_obj_center(label);

}

void setup(void)
{
    Serial.begin(115200);

    // Begin LilyGo amoled board class , only 1.47 inch amoled has sensor
    amoled.beginAMOLED_147();

    // Register lvgl helper
    beginLvglHelper(amoled);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);

    label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_timer_create(timer_event_handler, 1000, NULL);
}

void loop()
{
    lv_task_handler();
    delay(5);
}

