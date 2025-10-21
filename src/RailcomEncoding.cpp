#include "RailcomEncoding.h"

namespace RailcomEncoding {

// Lookup table for 4-out-of-8 encoding (from RCN-217, Table 2)
static const uint8_t PROGMEM encoding_table[64] = {
    0b10101100, 0b10101010, 0b10101001, 0b10100101, 0b10100011, 0b10100110, 0b10011100, 0b10001110,
    0b10001101, 0b10010101, 0b10010011, 0b10010110, 0b10011001, 0b10011010, 0b10011100, 0b1001110,
    0b10110010, 0b10110100, 0b10111000, 0b01110100, 0b01110010, 0b01101100, 0b01101010, 0b01101001,
    0b01100101, 0b01100011, 0b01100110, 0b01011100, 0b01011010, 0b01011001, 0b01010101, 0b01010011,
    0b01010110, 0b01001110, 0b01001101, 0b01001011, 0b01000111, 0b01110001, 0b11101000, 0b11100100,
    0b11100010, 0b11010001, 0b11001001, 0b11000101, 0b11011000, 0b11010100, 0b11010010, 0b11001010,
    0b11000110, 0b11001100, 0b01111000, 0b00010111, 0b00011011, 0b00011101, 0b00011110, 0b00101110,
    0b00110110, 0b00111010, 0b00100111, 0b00101011, 0b00101101, 0b00110101, 0b00111001, 0b00110011
};

uint8_t encode4of8(uint8_t value) {
    if (value > 63) {
        return 0; // Invalid input
    }
    return pgm_read_byte(&encoding_table[value]);
}

int16_t decode4of8(uint8_t encodedByte) {
    // Check for special values first
    if (encodedByte == ACK1 || encodedByte == ACK2) {
        return 0x100; // Special code for ACK
    }
    if (encodedByte == NACK) {
        return 0x101; // Special code for NACK
    }

    // Check Hamming weight (must be 4)
    uint8_t weight = 0;
    for (int i = 0; i < 8; ++i) {
        if ((encodedByte >> i) & 1) {
            weight++;
        }
    }
    if (weight != 4) {
        return -1; // Invalid encoding
    }

    // Search the table for the encoded value
    for (int i = 0; i < 64; ++i) {
        if (pgm_read_byte(&encoding_table[i]) == encodedByte) {
            return i;
        }
    }

    return -1; // Not found in table
}

} // namespace RailcomEncoding
