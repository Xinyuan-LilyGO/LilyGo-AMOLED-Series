/**
 * @file      arc.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 *
 */
#if defined(LILYGO_TWRITSTBAND_S3)
#include <LilyGo_Wristband.h> //To use LilyGo Wristband S3, please include <LilyGo_Wristband.h>
#elif defined(LILYGO_TDISPLAY_AMOLED_SERIES)
#include <LilyGo_AMOLED.h>      //To use LilyGo AMOLED series screens, please include <LilyGo_AMOLED.h>
#endif
#include <LV_Helper.h>

extern "C" {
    void lv_example_arc_1(void);
    void lv_example_arc_2(void);
};

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
    lv_example_arc_1();
    // lv_example_arc_2();
}


void loop()
{
    lv_task_handler();
    delay(5);
}
