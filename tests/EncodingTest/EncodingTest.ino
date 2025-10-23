// Copyright 2024 Jens-Michael Hoffmann
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <AUnit.h>
#include "RailcomEncoding.h"

// According to RCN-217, chapter 2.5, the Railcom signal is secured with a
// 4-out-of-8 coding. This test verifies that all 6-bit values can be encoded
// and decoded correctly.
test(Encoding, encodeDecodeRoundtrip) {
    for (uint8_t i = 0; i < 64; ++i) {
        uint8_t encoded = RailcomEncoding::encode4of8(i);
        int16_t decoded = RailcomEncoding::decode4of8(encoded);
        assertEqual(decoded, i);
    }
}

// This test verifies that bytes with an incorrect Hamming weight (i.e. not
// exactly four '1's) are rejected by the decoder. It also checks that a byte
// with the correct weight but not in the lookup table is rejected.
test(Encoding, decodeInvalid) {
    // Test bytes with incorrect Hamming weight
    assertEqual(RailcomEncoding::decode4of8(0b00000000), -1); // Weight 0
    assertEqual(RailcomEncoding::decode4of8(0b11111111), -1); // Weight 8
    assertEqual(RailcomEncoding::decode4of8(0b10101010), -1); // Weight 4, but not in table
}

// This test verifies that the special values for ACK1, ACK2, and NACK are
// decoded correctly. These are special signals that do not represent regular
// data values.
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
