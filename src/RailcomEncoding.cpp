#include "RailcomEncoding.h"
#include <map>

namespace RailcomEncoding {

// As defined in RCN-217, Table 2
const uint8_t ENCODE_TABLE[] = {
    0x0F, 0x17, 0x1B, 0x1D, 0x1E, 0x27, 0x2B, 0x2D, 0x2E, 0x33, 0x35, 0x36,
    0x39, 0x3A, 0x3C, 0x47, 0x4B, 0x4D, 0x4E, 0x53, 0x55, 0x56, 0x59, 0x5A,
    0x5C, 0x63, 0x65, 0x66, 0x69, 0x6A, 0x6C, 0x71, 0x72, 0x74, 0x78, 0x87,
    0x8B, 0x8D, 0x8E, 0x93, 0x95, 0x96, 0x99, 0x9A, 0x9C, 0xA3, 0xA5, 0xA6,
    0xA9, 0xAA, 0xAC, 0xB1, 0xB2, 0xB4, 0xB8, 0xC3, 0xC5, 0xC6, 0xC9, 0xCA,
    0xCC, 0xD1, 0xD2, 0xD4, 0xD8, 0xE1, 0xE2, 0xE4, 0xE8, 0xF0
};

uint8_t encode4of8(uint8_t value) {
    if (value > 63) return 0;
    return ENCODE_TABLE[value];
}

// Create a reverse mapping of the ENCODE_TABLE for fast decoding
static std::map<uint8_t, int16_t> DECODE_TABLE;
static bool DECODE_TABLE_INITIALIZED = false;

void initializeDecodeTable() {
    if (DECODE_TABLE_INITIALIZED) return;
    for (int i = 0; i < sizeof(ENCODE_TABLE); ++i) {
        DECODE_TABLE[ENCODE_TABLE[i]] = i;
    }
    DECODE_TABLE_INITIALIZED = true;
}

int16_t decode4of8(uint8_t value) {
    if (!DECODE_TABLE_INITIALIZED) {
        initializeDecodeTable();
    }
    auto it = DECODE_TABLE.find(value);
    if (it != DECODE_TABLE.end()) {
        return it->second;
    }
    return -1; // Not found
}

uint8_t crc8(const uint8_t* data, size_t len, uint8_t init) {
    uint8_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31; // Polynomial 0x31 (reversed 0x8C)
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

std::vector<uint8_t> encodeDatagram(RailcomID id, uint64_t payload, uint8_t payloadBits) {
    uint8_t totalBits = 4 + payloadBits;
    // Round up to the nearest 6 bits
    if (totalBits % 6 != 0) {
        totalBits = ((totalBits / 6) + 1) * 6;
    }
    uint8_t numBytes = totalBits / 6;
    uint64_t data = ((uint64_t)static_cast<uint8_t>(id) << payloadBits) | payload;

    std::vector<uint8_t> encodedBytes;
    int currentBit = totalBits - 6;
    for (int i = 0; i < numBytes; ++i) {
        uint8_t chunk = (data >> currentBit) & 0x3F;
        encodedBytes.push_back(encode4of8(chunk));
        currentBit -= 6;
    }
    return encodedBytes;
}

std::vector<uint8_t> encodeServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    uint16_t payload = (accessoryAddress & 0x7FF) | (isExtended ? 0x800 : 0x000);
    return encodeDatagram(RailcomID::SRQ, payload, 12);
}

}
