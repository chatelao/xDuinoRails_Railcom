#ifndef RAILCOM_ENCODING_H
#define RAILCOM_ENCODING_H

#include <Arduino.h>

namespace RailcomEncoding {
    // Encodes 6 bits of data into a 4-out-of-8 encoded byte.
    uint8_t encode4of8(uint8_t value);
}

#endif // RAILCOM_ENCODING_H
