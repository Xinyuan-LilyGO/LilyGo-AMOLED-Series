/**
 * @file      AdjustBrightness.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2024-03-19
 * @note      Brightness adjustable range 0 ~ 255
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;

static lv_obj_t *slider_label;

static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    uint8_t level = (uint8_t)lv_slider_get_value(slider);
    lv_label_set_text_fmt(slider_label, "%u", level);
    amoled.setBrightness(level);
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


void setup()
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

    beginLvglHelper(amoled);

    /*Create a slider in the center of the display*/
    lv_obj_t *slider = lv_slider_create(lv_scr_act());

    // Set brightness adjustable range 0 ~ 255
    lv_slider_set_range(slider, 0, 255);
    lv_obj_center(slider);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a label below the slider*/
    slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "0%");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


void loop()
{
    lv_task_handler();
    delay(5);
}
