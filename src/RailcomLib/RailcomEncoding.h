#ifndef RAILCOM_ENCODING_H
#define RAILCOM_ENCODING_H

#include <Arduino.h>

namespace RailcomEncoding {
    uint8_t encode4of8(uint8_t value);
    uint8_t crc8(const uint8_t* data, size_t len, uint8_t init = 0);
}

#endif // RAILCOM_ENCODING_H
