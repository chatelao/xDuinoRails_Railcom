#include "RailcomDecoding.h"
#include "Railcom.h" // For ACK/NACK constants

namespace RailcomEncoding {
    // Make the encoding table visible to this file.
    extern const uint8_t PROGMEM encoding_table[64];
}

namespace RailcomDecoding {

int16_t decode4of8(uint8_t encodedByte) {
    if (encodedByte == RAILCOM_ACK1 || encodedByte == RAILCOM_ACK2) {
        return 0x100; // Special code for ACK
    }
    if (encodedByte == RAILCOM_NACK) {
        return 0x101; // Special code for NACK
    }

    uint8_t weight = 0;
    for (int i = 0; i < 8; ++i) {
        if ((encodedByte >> i) & 1) weight++;
    }
    if (weight != 4) {
        return -1; // Invalid Hamming weight
    }

    for (int i = 0; i < 64; ++i) {
        if (pgm_read_byte(&RailcomEncoding::encoding_table[i]) == encodedByte) {
            return i;
        }
    }

    return -1; // Not found
}

} // namespace RailcomDecoding
