/**
 * @file      chart.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-08-09
 *
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

extern "C" {
    void lv_example_chart_1(void);
    void lv_example_chart_2(void);
    void lv_example_chart_3(void);
    void lv_example_chart_4(void);
    void lv_example_chart_5(void);
    void lv_example_chart_6(void);
    void lv_example_chart_7(void);
    void lv_example_chart_8(void);
    void lv_example_chart_9(void);
};

void setup()
{
    Serial.begin(115200);

    // Begin LilyGo amoled board class
    amoled.beginAMOLED_147();
    // amoled.beginAMOLED_191();

    beginLvglHelper();

    //Tips : Select a separate function to see the effect
    // lv_example_chart_1();
    lv_example_chart_2();
    // lv_example_chart_3();
    // lv_example_chart_4();
    // lv_example_chart_5();
    // lv_example_chart_6();
    // lv_example_chart_7();
    // lv_example_chart_8();
    // lv_example_chart_9();
}


void loop()
{
    lv_task_handler();
    delay(5);
}
