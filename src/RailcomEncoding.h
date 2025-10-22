#ifndef RAILCOM_ENCODING_H
#define RAILCOM_ENCODING_H

#include <Arduino.h>

namespace RailcomEncoding {

// Encodes 6 bits of data into a 4-out-of-8 encoded byte.
// Returns the encoded byte, or 0 if the input value is out of range.
uint8_t encode4of8(uint8_t value);

// Decodes a 4-out-of-8 encoded byte into 6 bits of data.
// Returns the decoded value, or -1 if the encoded byte is invalid.
int16_t decode4of8(uint8_t encodedByte);

// Calculates the RCN-218 CRC8.
uint8_t crc8(const uint8_t* data, size_t len, uint8_t init = 0);

// Special values for ACK/NACK
const uint8_t ACK1 = 0b11110000;
const uint8_t ACK2 = 0b00001111;
const uint8_t NACK = 0b00111100;

} // namespace RailcomEncoding

#endif // RAILCOM_ENCODING_H
