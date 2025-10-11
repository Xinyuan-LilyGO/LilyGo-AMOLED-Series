/**
 * @file      QWIIC_I2C_Scan.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  ShenZhen XinYuan Electronic Technology Co., Ltd
 * @date      2025-10-11
 * @note      Use the UART port to configure the second I2C port
 */
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <Arduino.h>
#include <Wire.h>

// The default is the UART port, which is used to configure the second I2C port
#define QWIIC_UART_TX_PIN       (43)
#define QWIIC_UART_RX_PIN       (44)

uint32_t deviceScan(TwoWire &_port)
{
    uint8_t err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++) {
        _port.beginTransmission(addr);
        err = _port.endTransmission();
        if (err == 0) {
            Serial.print("I2C device found at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.print(addr, HEX);
            Serial.println(" !");
            nDevices++;
        } else if (err == 4) {
            Serial.print("Unknow error at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.println(addr, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("Done\n");
    return nDevices;
}


void setup()
{
    Serial.begin(115200);
    while (!Serial) {}

    Serial.println("QWIIC I2C Scanner");

    // Change the Qwiic UART port to an I2C port
    Wire1.begin(QWIIC_UART_TX_PIN, QWIIC_UART_RX_PIN);

    Serial.println("Please note that the UART port does not have an I2C pull-up resistor and is configured for I2C function.");

}

void loop()
{
    Serial.println("Scanning I2C bus 1 ...");
    deviceScan(Wire1);
    delay(5000);
}

