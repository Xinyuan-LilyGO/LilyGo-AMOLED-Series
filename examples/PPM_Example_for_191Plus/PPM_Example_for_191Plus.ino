/**
 * @file      PPM_Example_for_191Plus.ino
 * @note      Example for AMOLED 1.91 Plus using SensorLib PMIC interfaces.
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>

LilyGo_Class amoled;
lv_obj_t *label1;
uint32_t nextUpdate;

static float readValue(PmicAdcBase &adc, PmicAdcBase::Channel channel)
{
    float value = 0;
    adc.read(channel, value);
    return value;
}

void setup()
{
    Serial.begin(115200);
    if (!amoled.beginAMOLED_191_SPI()) {
        while (true) {
            Serial.println("Unable to initialize AMOLED 1.91 Plus PMIC");
            delay(1000);
        }
    }

    beginLvglHelper(amoled);
    PmicBase *pmu = amoled.pmic();
    auto &charger = *pmu->getCharger();
    auto &adc = pmu->getAdc();

    charger.setChargeVoltage(4208);
    charger.setPreChargeCurrent(64);
    charger.setFastChargeCurrent(832);
    charger.enableCharging(false);

    Serial.printf("Configured charge current: %u mA\n", charger.getFastChargeCurrent());
    label1 = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label1, true);
    lv_obj_center(label1);
}

void loop()
{
    if (millis() >= nextUpdate) {
        PmicBase *pmu = amoled.pmic();
        const auto status = pmu->getCharger()->getStatus();
        const float vbus = readValue(pmu->getAdc(), PmicAdcBase::Channel::VBUS_VOLTAGE);
        const float battery = readValue(pmu->getAdc(), PmicAdcBase::Channel::BAT_VOLTAGE);
        const float system = readValue(pmu->getAdc(), PmicAdcBase::Channel::VSYS_VOLTAGE);

        lv_label_set_text_fmt(label1,
                              "Charging:%s\nUSB:%s\nVBUS:%u mV\nBattery:%u mV\nSystem:%u mV\nCharge current:%u mA",
                              status.charging ? "#00ff00 YES" : "#ff0000 NO",
                              status.vbusPresent ? "#00ff00 Connected" : "#ff0000 Disconnected",
                              static_cast<unsigned>(vbus),
                              static_cast<unsigned>(battery),
                              static_cast<unsigned>(system),
                              pmu->getCharger()->getFastChargeCurrent());
        Serial.printf("VBUS=%u mV VBAT=%u mV VSYS=%u mV charging=%s\n",
                      static_cast<unsigned>(vbus), static_cast<unsigned>(battery),
                      static_cast<unsigned>(system), status.charging ? "yes" : "no");
        nextUpdate = millis() + 1000;
    }

    lv_task_handler();
    delay(5);
}
