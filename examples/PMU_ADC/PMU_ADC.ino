/**
 * @file      PMU_ADC.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xinyuan Electronic Technology Co., Ltd
 * @date      2023-09-09
 * @note      Sketch only for 1.47" screen with Power management unit
 *            PMIC APIs are provided by SensorLib.
 */

#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;
bool  pmu_flag = 0;
lv_obj_t *label1;
uint32_t lastMillis;

static const char *chargeStatusName(PmicChargerBase::ChargingStatus status)
{
    switch (status) {
    case PmicChargerBase::ChargingStatus::PRE_CHARGE:
        return "Pre-charge";
    case PmicChargerBase::ChargingStatus::FAST_CHARGE:
        return "Fast charge";
    case PmicChargerBase::ChargingStatus::TERMINATION:
        return "Charge done";
    default:
        return "No charge";
    }
}

void setup()
{
    Serial.begin(115200);

    // Only support  LilyGo  1.47 Inch AMOLED board
    amoled.beginAMOLED_147();

    beginLvglHelper(amoled);

    label1 = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label1, true);    /*Enable re-coloring by commands in the text*/
    lv_obj_center(label1);
}

void loop()
{
    if (lastMillis < millis()) {
        const auto status = amoled.AXP.charger().getStatus();
        float battery = 0;
        float vbus = 0;
        float system = 0;
        float percent = 0;
        auto &adc = amoled.AXP.getAdc();
        adc.read(PmicAdcBase::Channel::BAT_VOLTAGE, battery);
        adc.read(PmicAdcBase::Channel::VBUS_VOLTAGE, vbus);
        adc.read(PmicAdcBase::Channel::VSYS_VOLTAGE, system);
        adc.read(PmicAdcBase::Channel::BAT_PERCENTAGE, percent);
        lv_label_set_text_fmt(label1, "Charging:%s\nDischarge:%s\nUSB PlugIn:%s\nCHG state:%s\nBattery Voltage:%u mV\n USB Voltage:%u mV\n SYS Voltage:%u mV\n Battery Percent:%d%%",
                              status.charging ? "#00ff00 YES" : "#ff0000 NO",
                              status.batteryPresent && !status.vbusPresent && !status.charging ? "#00ff00 YES" : "#ff0000 NO",
                              status.vbusPresent ? "#00ff00 YES" : "#ff0000 NO",
                              chargeStatusName(status.chargingStatus),
                              static_cast<unsigned>(battery),
                              static_cast<unsigned>(vbus),
                              static_cast<unsigned>(system),
                              static_cast<int>(percent)
                             );
        lastMillis = millis() + 1000;
    }
    lv_task_handler();
    delay(5);
}

