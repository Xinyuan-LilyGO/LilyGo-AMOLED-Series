/**
 * @file      PMU_Interrupt.ino
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

void setFlag(void)
{
    pmu_flag = true;
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

    // Register interrupt Callback
    amoled.attachPMU(setFlag);

    // Disable all interrupts
    amoled.diablePMUInterrupt(XPOWERS_AXP2101_ALL_IRQ);

    // Print interrupt register
    amoled.printIntRegister(&Serial);

    // Clear all interrupt flags
    amoled.clearPMU();

    // Enable the required interrupt function
    amoled.enablePMUInterrupt(
        XPOWERS_AXP2101_BAT_INSERT_IRQ    | XPOWERS_AXP2101_BAT_REMOVE_IRQ      |   //BATTERY
        XPOWERS_AXP2101_VBUS_INSERT_IRQ   | XPOWERS_AXP2101_VBUS_REMOVE_IRQ     |   //VBUS
        XPOWERS_AXP2101_PKEY_SHORT_IRQ    | XPOWERS_AXP2101_PKEY_LONG_IRQ       |   //POWER KEY
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ  | XPOWERS_AXP2101_BAT_CHG_START_IRQ       //CHARGE
        // XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ | XPOWERS_AXP2101_PKEY_POSITIVE_IRQ   |   //POWER KEY
    );

    amoled.enablePMUInterrupt(XPOWERS_AXP2101_BAT_NOR_UNDER_TEMP_IRQ);

    amoled.enablePMUInterrupt(XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_NEGATIVE_IRQ);

    amoled.enablePMUInterrupt(XPOWERS_AXP2101_BAT_OVER_VOL_IRQ);


    lv_label_set_text(label1, "Wait PMU IRQ!");

}

void loop()
{

    if (pmu_flag) {

        pmu_flag = false;

        // Get amoled Interrupt Status Register
        uint32_t status = amoled.readPMU();
        Serial.print("STATUS => HEX:");
        Serial.print(status, HEX);
        Serial.print(" BIN:");
        Serial.println(status, BIN);

        if (amoled.isDropWarningLevel2Irq()) {
            Serial.println("isDropWarningLevel2");
            lv_label_set_text(label1, "isDropWarningLevel2");
        }
        if (amoled.isDropWarningLevel1Irq()) {
            Serial.println("isDropWarningLevel1");
            lv_label_set_text(label1, "isDropWarningLevel1");
        }
        if (amoled.isGaugeWdtTimeoutIrq()) {
            Serial.println("isWdtTimeout");
            lv_label_set_text(label1, "isWdtTimeout");
        }
        if (amoled.isBatChargerOverTemperatureIrq()) {
            Serial.println("isBatChargeOverTemperature");
            lv_label_set_text(label1, "isBatChargeOverTemperature");
        }
        if (amoled.isBatWorkOverTemperatureIrq()) {
            Serial.println("isBatWorkOverTemperature");
            lv_label_set_text(label1, "isBatWorkOverTemperature");
        }
        if (amoled.isBatWorkUnderTemperatureIrq()) {
            Serial.println("isBatWorkUnderTemperature");
            lv_label_set_text(label1, "isBatWorkUnderTemperature");
        }
        if (amoled.isVbusInsertIrq()) {
            Serial.println("isVbusInsert");
            lv_label_set_text(label1, "isVbusInsert");
        }
        if (amoled.isVbusRemoveIrq()) {
            Serial.println("isVbusRemove");
            lv_label_set_text(label1, "isVbusRemove");
        }
        if (amoled.isBatInsertIrq()) {
            Serial.println("isBatInsert");
            lv_label_set_text(label1, "isBatInsert");
        }
        if (amoled.isBatRemoveIrq()) {
            Serial.println("isBatRemove");
            lv_label_set_text(label1, "isBatRemove");
        }
        if (amoled.isPekeyShortPressIrq()) {
            Serial.println("isPekeyShortPress");
            lv_label_set_text(label1, "isPekeyShortPress");
        }
        if (amoled.isPekeyLongPressIrq()) {
            Serial.println("isPekeyLongPress");
            lv_label_set_text(label1, "isPekeyLongPress");
        }
        if (amoled.isPekeyNegativeIrq()) {
            Serial.println("isPekeyNegative");
            lv_label_set_text(label1, "isPekeyNegative");
        }
        if (amoled.isPekeyPositiveIrq()) {
            Serial.println("isPekeyPositive");
            lv_label_set_text(label1, "isPekeyPositive");
        }
        if (amoled.isWdtExpireIrq()) {
            Serial.println("isWdtExpire");
            lv_label_set_text(label1, "isWdtExpire");
        }
        if (amoled.isLdoOverCurrentIrq()) {
            Serial.println("isLdoOverCurrentIrq");
            lv_label_set_text(label1, "isLdoOverCurrentIrq");
        }
        if (amoled.isBatfetOverCurrentIrq()) {
            Serial.println("isBatfetOverCurrentIrq");
            lv_label_set_text(label1, "isBatfetOverCurrentIrq");
        }

        if (amoled.isBatteryConnect()) {
            if (amoled.isBatChagerDoneIrq()) {
                Serial.println("isBatChagerDone");
                lv_label_set_text(label1, "isBatChagerDone");
            }
            if (amoled.isBatChagerStartIrq()) {
                Serial.println("isBatChagerStart");
                lv_label_set_text(label1, "isBatChagerStart");
            }
            if (amoled.isChagerOverTimeoutIrq()) {
                Serial.println("isChagerOverTimeout");
                lv_label_set_text(label1, "isChagerOverTimeout");
            }
        }

        if (amoled.isBatDieOverTemperatureIrq()) {
            Serial.println("isBatDieOverTemperature");
            lv_label_set_text(label1, "isBatDieOverTemperature");
        }

        if (amoled.isBatOverVoltageIrq()) {
            Serial.println("isBatOverVoltage");
            lv_label_set_text(label1, "isBatOverVoltage");
        }

        // Clear amoled Interrupt Status Register
        amoled.clearPMU();

        // Print AXP2101 interrupt control register
        amoled.printIntRegister(&Serial);

    }
    lv_task_handler();
    delay(5);
}

