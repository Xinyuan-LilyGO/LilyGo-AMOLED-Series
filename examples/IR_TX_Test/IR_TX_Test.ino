/*
 * FactoryV2 AMOLED Plus Shield: standalone IR transmitter test.
 *
 * Only the T-Display-S3 AMOLED Plus with the matching Shield is supported.
 * Other AMOLED Series boards do not have the required IR transmit path.
 *
 * The sketch sends one valid 32-bit NEC frame every second and reports each
 * frame on the serial monitor at 115200.
 */

#include <Arduino.h>
#include <IRsend.h>

namespace {
constexpr uint16_t kIrPin = 1;
constexpr uint16_t kIrSelectPin = 39;
constexpr uint32_t kNecTestCode = 0x20DF10EFUL;
}

IRsend irSender(kIrPin);
uint32_t sentFrames = 0;
uint32_t lastSendMs = 0;

void selectTransmitPath()
{
    pinMode(kIrSelectPin, OUTPUT);
    digitalWrite(kIrSelectPin, LOW);  // Shield: LOW selects IR TX.
    delay(5);                          // Allow the analog switch to settle.
    irSender.begin();
}

void releaseTransmitPath()
{
    // sendNEC() already ends with an IR space. Force the default inactive
    // level before returning the shared line to high impedance.
    pinMode(kIrPin, OUTPUT);
    digitalWrite(kIrPin, LOW);
    pinMode(kIrPin, INPUT);
    digitalWrite(kIrSelectPin, HIGH);  // Shield: HIGH selects IR RX/idle.
}

void sendTestFrame()
{
    selectTransmitPath();

    // IRremoteESP8266 creates the carrier in software. Keep this short frame
    // contiguous so unrelated interrupts cannot stretch NEC marks/spaces.
    noInterrupts();
    irSender.sendNEC(kNecTestCode, 32);
    interrupts();

    releaseTransmitPath();
    ++sentFrames;
    Serial.printf("[ir-tx] sent NEC 0x%08lX, frame=%lu\n",
                  (unsigned long)kNecTestCode, (unsigned long)sentFrames);
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    releaseTransmitPath();
    Serial.println();
    Serial.println("[ir-tx] standalone FactoryV2 IR transmitter test");
    Serial.printf("[ir-tx] GPIO=%u select=%u code=0x%08lX bits=32\n",
                  kIrPin, kIrSelectPin, (unsigned long)kNecTestCode);
    Serial.println("[ir-tx] sending one NEC frame every 1000 ms");
}

void loop()
{
    const uint32_t now = millis();
    if ((uint32_t)(now - lastSendMs) >= 1000) {
        lastSendMs = now;
        sendTestFrame();
    }
    delay(2);
}
