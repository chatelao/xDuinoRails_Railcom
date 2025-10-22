#include <ArduinoUnit.h>
#include "Railcom.h"

test(Encoding_encodeDecodeRoundtrip) {
    for (uint8_t i = 0; i < 64; ++i) {
        uint8_t encoded = Railcom::encode4of8(i);
        int16_t decoded = Railcom::decode4of8(encoded);
        assertEqual(decoded, i);
    }
}

test(Encoding_decodeInvalid) {
    // Test bytes with incorrect Hamming weight
    assertEqual(Railcom::decode4of8(0b00000000), -1); // Weight 0
    assertEqual(Railcom::decode4of8(0b11111111), -1); // Weight 8
    assertEqual(Railcom::decode4of8(0b10101010), -1); // Weight 4, but not in table
}

test(Encoding_decodeSpecialValues) {
    assertEqual(Railcom::decode4of8(Railcom::ACK1), 0x100);
    assertEqual(Railcom::decode4of8(Railcom::ACK2), 0x100);
    assertEqual(Railcom::decode4of8(Railcom::NACK), 0x101);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
}

void loop() {
  Test::run();
}
