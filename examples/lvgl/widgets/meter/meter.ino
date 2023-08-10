/**
 * @file      meter.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 *
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

extern "C" {
    void lv_example_meter_1(void);
    void lv_example_meter_2(void);
    void lv_example_meter_3(void);
    void lv_example_meter_4(void);
};

void setup()
{
    Serial.begin(115200);

    // Begin LilyGo amoled board class
    amoled.beginAMOLED_147();
    // amoled.beginAMOLED_191();

    beginLvglHelper();

    //Tips : Select a separate function to see the effect
    lv_example_meter_1();
    // lv_example_meter_2();
    // lv_example_meter_3();
    // lv_example_meter_4();
}


void loop()
{
    lv_task_handler();
    delay(5);
}
