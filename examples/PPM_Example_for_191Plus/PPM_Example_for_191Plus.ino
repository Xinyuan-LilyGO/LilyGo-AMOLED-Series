/**
 * @file      PPM_Example_for_191Plus.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2024  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2024-12-12
 * @note      Examples only apply to AMOLED 1.91 Plus
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

    //  * @note      Examples only apply to AMOLED 1.91 Plus
    bool rslt = amoled.beginAMOLED_191_SPI();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }

    beginLvglHelper(amoled);

    // Set the charging target voltage, Range:3840 ~ 4608mV ,step:16 mV
    amoled.BQ.setChargeTargetVoltage(4208);

    // Set the precharge current , Range: 64mA ~ 1024mA ,step:64mA
    amoled.BQ.setPrechargeCurr(64);

    // The premise is that Limit Pin is disabled, or it will only follow the maximum charging current set by Limit Pin.
    // Set the charging current , Range:0~5056mA ,step:64mA
    amoled.BQ.setChargerConstantCurr(832);

    // Get the set charging current
    amoled.BQ.getChargerConstantCurr();
    Serial.printf("getChargerConstantCurr: %d mA\n", amoled.BQ.getChargerConstantCurr());


    // To obtain voltage data, the ADC must be enabled first
    amoled.BQ.enableMeasure();

    // Turn on charging function
    // If the battery does not exist, the data may be confused when the charging function is turned on.
    // The chip cannot determine whether the battery exists
    // amoled.BQ.enableCharge();

    // Turn off charging function
    // If USB is used as the only power input, it is best to turn off the charging function,
    // otherwise the VSYS power supply will have a sawtooth wave, affecting the discharge output capability.
    amoled.BQ.disableCharge();

    // The OTG function needs to enable OTG, and set the OTG control pin to HIGH
    // After OTG is enabled, if an external power supply is plugged in, OTG will be turned off
    // amoled.BQ.enableOTG();
    // amoled.BQ.disableOTG();

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
        Serial.print(amoled.BQ.isVbusIn() ? "Connected" : "Disconnect"); Serial.print("\t");
        Serial.print(amoled.BQ.getVbusVoltage()); Serial.print("\t");
        Serial.print(amoled.BQ.getBattVoltage()); Serial.print("\t");
        Serial.print(amoled.BQ.getSystemVoltage()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(amoled.BQ.getBusStatus(), HEX); Serial.print("\t");
        Serial.print(amoled.BQ.getBusStatusString()); Serial.print("\t");
        Serial.print("0x");
        Serial.print(amoled.BQ.chargeStatus(), HEX); Serial.print("\t");
        Serial.print(amoled.BQ.getChargeStatusString()); Serial.print("\t");

        Serial.print(amoled.BQ.getChargeTargetVoltage()); Serial.print("\t");
        Serial.print(amoled.BQ.getChargeCurrent()); Serial.print("\t");
        Serial.print(amoled.BQ.getPrechargeCurr()); Serial.print("\t");
        Serial.print(amoled.BQ.getNTCStatus()); Serial.print("\t");
        Serial.print(amoled.BQ.getNTCStatusString()); Serial.print("\t");

        lv_label_set_text_fmt(label1, "Charging:%s\nUSB PlugIn:%s\nCHG state:%s\nBattery Voltage:%u mV\n USB Voltage:%u mV\n SYS Voltage:%u mV\n",
                              amoled.BQ.isCharging() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.BQ.isVbusIn() ? "#00ff00 YES" : "#ff0000 NO",
                              amoled.BQ.getChargeStatusString(),
                              amoled.BQ.getBattVoltage(),
                              amoled.BQ.getVbusVoltage(),
                              amoled.BQ.getSystemVoltage()
                             );

        Serial.println();
        Serial.println();
        cycleInterval = millis() + 1000;
    }

    lv_task_handler();
    delay(5);
}





