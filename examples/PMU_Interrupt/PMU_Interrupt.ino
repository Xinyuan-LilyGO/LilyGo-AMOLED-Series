/**
 * @file      PMU_Interrupt.ino
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
    amoled.disablePMUInterrupt(AXP2101Irq::IRQ_ALL_MASK);

    // Clear all interrupt flags
    amoled.clearPMU();

    // Enable the required interrupt function
    amoled.enablePMUInterrupt(
        AXP2101Irq::IRQ_BAT_INSERT | AXP2101Irq::IRQ_BAT_REMOVE |
        AXP2101Irq::IRQ_VBUS_INSERT | AXP2101Irq::IRQ_VBUS_REMOVE |
        AXP2101Irq::IRQ_PEKEY_SHORT_PRESS | AXP2101Irq::IRQ_PEKEY_LONG_PRESS |
        AXP2101Irq::IRQ_BAT_CHG_DONE | AXP2101Irq::IRQ_BAT_CHG_START
    );

    amoled.enablePMUInterrupt(AXP2101Irq::IRQ_BAT_UNDER_TEMP_WORK);

    amoled.enablePMUInterrupt(AXP2101Irq::IRQ_PEKEY_SHORT_PRESS | AXP2101Irq::IRQ_PEKEY_NEGATIVE_EDGE);

    amoled.enablePMUInterrupt(AXP2101Irq::IRQ_BAT_OVER_VOLT);


    lv_label_set_text(label1, "Wait PMU IRQ!");

}

void loop()
{

    if (pmu_flag) {

        pmu_flag = false;

        const uint64_t status = amoled.readPMU();
        Serial.printf("AXP2101 IRQ status: 0x%06llX\n", static_cast<unsigned long long>(status));

        static const struct {
            uint64_t mask;
            const char *name;
        } events[] = {
            {AXP2101Irq::IRQ_WARNING_LEVEL2, "Battery warning level 2"},
            {AXP2101Irq::IRQ_WARNING_LEVEL1, "Battery warning level 1"},
            {AXP2101Irq::IRQ_GAUGE_WDT_TIMEOUT, "Gauge watchdog timeout"},
            {AXP2101Irq::IRQ_BAT_OVER_TEMP_WORK, "Battery over-temperature"},
            {AXP2101Irq::IRQ_BAT_UNDER_TEMP_WORK, "Battery under-temperature"},
            {AXP2101Irq::IRQ_VBUS_INSERT, "VBUS inserted"},
            {AXP2101Irq::IRQ_VBUS_REMOVE, "VBUS removed"},
            {AXP2101Irq::IRQ_BAT_INSERT, "Battery inserted"},
            {AXP2101Irq::IRQ_BAT_REMOVE, "Battery removed"},
            {AXP2101Irq::IRQ_PEKEY_SHORT_PRESS, "Power key short press"},
            {AXP2101Irq::IRQ_PEKEY_LONG_PRESS, "Power key long press"},
            {AXP2101Irq::IRQ_PEKEY_NEGATIVE_EDGE, "Power key released"},
            {AXP2101Irq::IRQ_PEKEY_POSITIVE_EDGE, "Power key pressed"},
            {AXP2101Irq::IRQ_WDT_EXPIRE, "Watchdog expired"},
            {AXP2101Irq::IRQ_LDO_OVER_CURR, "LDO over-current"},
            {AXP2101Irq::IRQ_BATFET_OVER_CURR, "BATFET over-current"},
            {AXP2101Irq::IRQ_BAT_CHG_DONE, "Charge complete"},
            {AXP2101Irq::IRQ_BAT_CHG_START, "Charging started"},
            {AXP2101Irq::IRQ_CHG_TIMEOUT, "Charge timeout"},
            {AXP2101Irq::IRQ_DIE_OVER_TEMP, "PMIC over-temperature"},
            {AXP2101Irq::IRQ_BAT_OVER_VOLT, "Battery over-voltage"},
        };
        for (const auto &event : events) {
            if (AXP2101Irq::isIrqSet(event.mask, status)) {
                Serial.println(event.name);
                lv_label_set_text(label1, event.name);
            }
        }

        // Clear amoled Interrupt Status Register
        amoled.clearPMU();

    }
    lv_task_handler();
    delay(5);
}

