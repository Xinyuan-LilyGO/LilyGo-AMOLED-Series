/**
 * @file      label.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 *
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

extern "C" {
    void lv_example_label_1(void);
    void lv_example_label_2(void);
    void lv_example_label_3(void);
    void lv_example_label_4(void);
    void lv_example_label_5(void);
};

void setup()
{
    Serial.begin(115200);

    // Begin LilyGo  1.47 Inch AMOLED board class
    // amoled.beginAMOLED_147();


    // Begin LilyGo  1.91 Inch AMOLED board class
    // amoled.beginAMOLED_191();

    // Automatically determine the access device
    amoled.beginAutomatic();

    beginLvglHelper();

    //Tips : Select a separate function to see the effect
    lv_example_label_1();
    // lv_example_label_2();
    // lv_example_label_3();
    // lv_example_label_4();
    // lv_example_label_5();
}


void loop()
{
    lv_task_handler();
    delay(5);
}
