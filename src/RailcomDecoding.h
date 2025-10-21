#ifndef RAILCOM_DECODING_H
#define RAILCOM_DECODING_H

#include <Arduino.h>

namespace RailcomDecoding {
    // Decodes a 4-out-of-8 encoded byte into 6 bits of data.
    // Returns the decoded value (0-63), or a special code for ACK/NACK, or -1 for an error.
    int16_t decode4of8(uint8_t encodedByte);
}

#endif // RAILCOM_DECODING_H
