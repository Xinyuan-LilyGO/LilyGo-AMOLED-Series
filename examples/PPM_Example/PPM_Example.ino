/**
 * @file      PPM_Example.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-08-15
 * @note      Examples only apply to T4-S3
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;

uint32_t cycleInterval;
bool pmu_irq = false;
lv_obj_t *label1;

void setup()
{
    Serial.begin(115200);
    while (!Serial);

    //  * @note      Examples only apply to T4-S3
    //  * @note      Examples only apply to T4-S3
    bool rslt = amoled.beginAMOLED_241();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }

    beginLvglHelper(amoled);

    // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
    amoled.XPowersPPM::setChargeTargetVoltage(4208);

    // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
    amoled.XPowersPPM::setPrechargeCurr(64);

    // The premise is that Limit Pin is disabled, or it will only follow the maximum charging current set by Limi tPin.
    // Set the charging current , Range:0~5056mA ,step:64mA
    amoled.XPowersPPM::setChargerConstantCurr(832);

    // Get the set charging current
    amoled.XPowersPPM::getChargerConstantCurr();
    Serial.printf("getChargerConstantCurr: %d mA\n", amoled.XPowersPPM::getChargerConstantCurr());


    // To obtain voltage data, the ADC must be enabled first
    amoled.XPowersPPM::enableADCMeasure();

    // Turn on charging function
    // If the battery does not exist, the data may be confused when the charging function is turned on.
    // The chip cannot determine whether the battery exists
    // amoled.XPowersPPM::enableCharge();

    // Turn off charging function
    // If USB is used as the only power input, it is best to turn off the charging function,
    // otherwise the VSYS power supply will have a sawtooth wave, affecting the discharge output capability.
    amoled.XPowersPPM::disableCharge();

    // The OTG function needs to enable OTG, and set the OTG control pin to HIGH
    // After OTG is enabled, if an external power supply is plugged in, OTG will be turned off
    // amoled.XPowersPPM::enableOTG();
    // amoled.XPowersPPM::disableOTG();

    // Turn off charge led
    // Without a battery connected, the status light will flash to indicate an error
    amoled.XPowersPPM::disableStatLed();

    // Turn on charge led
    // amoled.XPowersPPM::enableStatLed();

    label1 = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label1, true);    /*Enable re-coloring by commands in the text*/
    lv_obj_center(label1);
}


void loop()
{
    // If the battery does not exist, the data may be confused when the charging function is turned on.
    // The chip cannot determine whether the battery exists
    if (millis() > cycleInterval) {

        Serial.println("Sats        VBUS    VBAT   SYS    VbusStatus      String   ChargeStatus     String      TargetVoltage       ChargeCurrent       Precharge       NTCStatus           String");
        Serial.println("            (mV)    (mV)   (mV)   (HEX)                         (HEX)                    (mV)                 (mA)                   (mA)           (HEX)           ");
        Serial.println("--------------------------------------------------------------------------------------------------------------------------------");
        Serial.print(amoled.XPowersPPM::isVbusIn() ? "Connected" : "Disconnect"); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getVbusVoltage()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getBattVoltage()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getSystemVoltage()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(amoled.XPowersPPM::getBusStatus(), HEX); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getBusStatusString()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(amoled.XPowersPPM::chargeStatus(), HEX); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getChargeStatusString()); Serial.print("\t");

        Serial.print(amoled.XPowersPPM::getChargeTargetVoltage()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getChargeCurrent()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getPrechargeCurr()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getNTCStatus()); Serial.print("\t");
        Serial.print(amoled.XPowersPPM::getNTCStatusString()); Serial.print("\t");

        lv_label_set_text_fmt(label1, "Charging:%s\nUSB PlugIn:%s\nCHG state:%s\nBattery Voltage:%u mV\n USB Voltage:%u mV\n SYS Voltage:%u mV\n",
                              amoled.XPowersPPM::isCharging() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.XPowersPPM::isVbusIn() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.XPowersPPM::getChargeStatusString(),
                              amoled.XPowersPPM::getBattVoltage(),
                              amoled.XPowersPPM::getVbusVoltage(),
                              amoled.XPowersPPM::getSystemVoltage()
                             );

        Serial.println();
        Serial.println();
        cycleInterval = millis() + 1000;
    }

    lv_task_handler();
    delay(5);
}





