/*
 * FactoryV2 AMOLED Plus Shield: standalone IR receiver test.
 *
 * The receiver listens continuously and prints every decoded frame. A valid
 * FactoryV2 test frame is NEC 0x20DF10EF.
 */

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

namespace {
constexpr uint16_t kIrPin = 1;
constexpr uint16_t kIrSelectPin = 39;
constexpr uint32_t kNecTestCode = 0x20DF10EFUL;
}

IRrecv irReceiver(kIrPin);
decode_results results;

void selectReceivePath()
{
    pinMode(kIrSelectPin, OUTPUT);
    digitalWrite(kIrSelectPin, HIGH);  // Shield: HIGH selects IR RX.
    delay(5);                          // Allow the analog switch to settle.
    irReceiver.setTolerance(35);
    irReceiver.enableIRIn();
}

void printRawSummary(const decode_results &frame)
{
    Serial.printf("[ir-rx] rawlen=%u overflow=%s\n",
                  (unsigned)frame.rawlen, frame.overflow ? "yes" : "no");
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    selectReceivePath();
    Serial.println();
    Serial.println("[ir-rx] standalone FactoryV2 IR receiver test");
    Serial.printf("[ir-rx] GPIO=%u select=%u expected NEC 0x%08lX bits=32\n",
                  kIrPin, kIrSelectPin, (unsigned long)kNecTestCode);
    Serial.println("[ir-rx] listening...");
}

void loop()
{
    if (irReceiver.decode(&results)) {
        const bool necType = results.decode_type == NEC ||
                             results.decode_type == NEC_LIKE;
        const bool factoryFrame = necType && results.bits == 32 &&
                                  static_cast<uint32_t>(results.value) == kNecTestCode;

        Serial.print("[ir-rx] value=0x");
        serialPrintUint64(results.value, HEX);
        Serial.printf(" protocol=%s bits=%u",
                      typeToString(results.decode_type, results.repeat).c_str(),
                      (unsigned)results.bits);
        Serial.println(factoryFrame ? " MATCH_FACTORY_NEC" : "");
        printRawSummary(results);
        irReceiver.resume();
    }
    delay(2);
}
