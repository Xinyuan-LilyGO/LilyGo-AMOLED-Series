/**
 * @file      gif.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-05-01
 *
 */
/*
* Uncomment the following line to use LilyGo-T-Wristband-S3.
* Note that LILYGO_TDISPLAY_AMOLED_SERIES needs to be commented out.
* 取消下面这行注释,将使用LilyGo-T-Wristband-S3, 注意需要将 LILYGO_TDISPLAY_AMOLED_SERIES 注释掉
* */
// #define LILYGO_TWRITSTBAND_S3

/*
* Uncomment the following line to use the LilyGo-AMOLED series
* 取消下面这行注释,将使用LilyGo-AMOLED系列 , 注意需要将 LILYGO_TWRITSTBAND_S3 注释掉
* */
#define LILYGO_TDISPLAY_AMOLED_SERIES

#if defined(LILYGO_TWRITSTBAND_S3)
#include <LilyGo_Wristband.h> //To use LilyGo Wristband S3, please include <LilyGo_Wristband.h>
#elif defined(LILYGO_TDISPLAY_AMOLED_SERIES)
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#endif
#include <LV_Helper.h>

#if !LV_USE_GIF
#error "lvgl gid decoder library is not enable!"
#endif

LV_IMG_DECLARE(ellipse80);

LilyGo_Class amoled;

void setup()
{
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

    lv_obj_t *img = lv_gif_create(lv_scr_act());
    lv_gif_set_src(img, &ellipse80);
    lv_obj_center(img);
}



void loop()
{
    lv_task_handler();
    delay(5);
}

