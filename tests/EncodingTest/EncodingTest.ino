#include <AUnit.h>
#include "RailcomEncoding.h"

test(Encoding, encodeDecodeRoundtrip) {
    for (uint8_t i = 0; i < 64; ++i) {
        uint8_t encoded = RailcomEncoding::encode4of8(i);
        int16_t decoded = RailcomEncoding::decode4of8(encoded);
        assertEqual(decoded, i);
    }
}

test(Encoding, decodeInvalid) {
    // Test bytes with incorrect Hamming weight
    assertEqual(RailcomEncoding::decode4of8(0b00000000), -1); // Weight 0
    assertEqual(RailcomEncoding::decode4of8(0b11111111), -1); // Weight 8
    assertEqual(RailcomEncoding::decode4of8(0b10101010), -1); // Weight 4, but not in table
}

test(Encoding, decodeSpecialValues) {
    assertEqual(RailcomEncoding::decode4of8(RailcomEncoding::ACK1), 0x100);
    assertEqual(RailcomEncoding::decode4of8(RailcomEncoding::ACK2), 0x100);
    assertEqual(RailcomEncoding::decode4of8(RailcomEncoding::NACK), 0x101);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    TestRunner::run();
}

void loop() {
}
