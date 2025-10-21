#include <AUnit.h>
#include "Railcom.h"
#include "RailcomEncoding.h"
#include "RailcomDecoding.h"

test(EncodingDecoding, roundtrip) {
    for (uint8_t i = 0; i < 64; ++i) {
        uint8_t encoded = RailcomEncoding::encode4of8(i);
        int16_t decoded = RailcomDecoding::decode4of8(encoded);
        assertEqual(decoded, i);
    }
}

test(EncodingDecoding, decodeInvalid) {
    assertEqual(RailcomDecoding::decode4of8(0b00000000), -1);
    assertEqual(RailcomDecoding::decode4of8(0b11111111), -1);
}

test(EncodingDecoding, decodeSpecialValues) {
    assertEqual(RailcomDecoding::decode4of8(RAILCOM_ACK1), 0x100);
    assertEqual(RailcomDecoding::decode4of8(RAILCOM_ACK2), 0x100);
    assertEqual(RailcomDecoding::decode4of8(RAILCOM_NACK), 0x101);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    TestRunner::run();
}

void loop() {}
