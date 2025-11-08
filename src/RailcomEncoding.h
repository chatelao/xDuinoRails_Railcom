#ifndef RAILCOM_ENCODING_H
#define RAILCOM_ENCODING_H

#include <Arduino.h>
#include <vector>
#include "Railcom.h"

namespace RailcomEncoding {
    uint8_t encode4of8(uint8_t value);
    int16_t decode4of8(uint8_t value);
    uint8_t crc8(const uint8_t* data, size_t len, uint8_t init = 0);

    std::vector<uint8_t> encodeDatagram(RailcomID id, uint64_t payload, uint8_t payloadBits);
    std::vector<uint8_t> encodeServiceRequest(uint16_t accessoryAddress, bool isExtended);
}

#endif // RAILCOM_ENCODING_H
