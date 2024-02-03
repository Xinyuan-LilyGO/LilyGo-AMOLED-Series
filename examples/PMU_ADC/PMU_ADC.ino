/**
 * @file      PMU_ADC.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-09-09
 * @note      Sketch only for 1.47" screen with Power management unit
 *            For more PMU examples, please refer to https://github.com/lewisxhe/XPowersLib
 */

#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;
bool  pmu_flag = 0;
lv_obj_t *label1;
uint32_t lastMillis;

const char *chg_status[] = {
    "Tri ",
    "Pre",
    "Constant current",
    "Constant voltage",
    "Charge done",
    "No charge"
};

void setup()
{
    Serial.begin(115200);

    // Only support  LilyGo  1.47 Inch AMOLED board
    amoled.beginAMOLED_147();

    beginLvglHelper(amoled);

    //Enable or Disable PMU Feature
    amoled.enableBattDetection();
    // amoled.disableBattDetection();

    amoled.enableVbusVoltageMeasure();
    // amoled.disableVbusVoltageMeasure();

    amoled.enableBattVoltageMeasure();
    // amoled.disableBattVoltageMeasure();

    amoled.enableSystemVoltageMeasure();
    // amoled.disableSystemVoltageMeasure();

    label1 = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label1, true);    /*Enable re-coloring by commands in the text*/
    lv_obj_center(label1);
}

void loop()
{
    if (lastMillis < millis()) {
        uint8_t charge_status = amoled.getChargerStatus();
        lv_label_set_text_fmt(label1, "Charging:%s\nDischarge:%s\nUSB PlugIn:%s\nCHG state:%s\nBattery Voltage:%u mV\n USB Voltage:%u mV\n SYS Voltage:%u mV\n Battery Percent:%d%%",
                              amoled.isCharging() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.isDischarge() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.isVbusIn() ? "#00ff00 YES" : "#ff0000 NO",
                              chg_status[charge_status],
                              amoled.getBattVoltage(),
                              amoled.getVbusVoltage(),
                              amoled.getSystemVoltage(),
                              amoled.getBatteryPercent()
                             );
        lastMillis = millis() + 1000;
    }
    lv_task_handler();
    delay(5);
}

