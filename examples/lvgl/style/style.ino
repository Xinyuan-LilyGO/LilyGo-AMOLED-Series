/**
 * @file      style.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-08-09
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
#include "lv_example_style.h"

LilyGo_Class amoled;

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

    //Tips : Select a separate function to see the effect
    lv_example_style_1();
    // lv_example_style_2();
    // lv_example_style_3();
    // lv_example_style_4();
    // lv_example_style_5();
    // lv_example_style_6();
    // lv_example_style_7();
    // lv_example_style_8();
    // lv_example_style_9();
    // lv_example_style_10();
    // lv_example_style_11();
    // lv_example_style_12();
    // lv_example_style_13();
    // lv_example_style_14();
    // lv_example_style_15();
}


void loop()
{
    lv_task_handler();
    delay(5);
}
