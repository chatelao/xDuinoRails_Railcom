/**
 * @file RailcomEncoding.cpp
 * @brief Implementation of RailCom encoding, decoding, and CRC functions.
 */
#include "RailcomEncoding.h"
#include <map>

namespace RailcomEncoding {

/**
 * @brief The official 4-of-8 encoding table.
 * @details This table maps a 6-bit input value (the array index) to its
 *          corresponding 8-bit encoded output value.
 * @see RCN-217, Table 2
 */
const uint8_t ENCODE_TABLE[] = {
    0x0F, 0x17, 0x1B, 0x1D, 0x1E, 0x27, 0x2B, 0x2D, 0x2E, 0x33, 0x35, 0x36,
    0x39, 0x3A, 0x3C, 0x47, 0x4B, 0x4D, 0x4E, 0x53, 0x55, 0x56, 0x59, 0x5A,
    0x5C, 0x63, 0x65, 0x66, 0x69, 0x6A, 0x6C, 0x71, 0x72, 0x74, 0x78, 0x87,
    0x8B, 0x8D, 0x8E, 0x93, 0x95, 0x96, 0x99, 0x9A, 0x9C, 0xA3, 0xA5, 0xA6,
    0xA9, 0xAA, 0xAC, 0xB1, 0xB2, 0xB4, 0xB8, 0xC3, 0xC5, 0xC6, 0xC9, 0xCA,
    0xCC, 0xD1, 0xD2, 0xD4, 0xD8, 0xE1, 0xE2, 0xE4, 0xE8, 0xF0
};

/**
 * @brief Encodes a 6-bit value to an 8-bit value using the ENCODE_TABLE.
 * @param value The 6-bit value (0-63).
 * @return The 8-bit encoded value.
 */
uint8_t encode4of8(uint8_t value) {
    if (value > 63) return 0;
    return ENCODE_TABLE[value];
}

/**
 * @brief A reverse lookup table for decoding, generated at runtime.
 */
static std::map<uint8_t, int16_t> DECODE_TABLE;
/**
 * @brief Flag to ensure the decode table is only initialized once.
 */
static bool DECODE_TABLE_INITIALIZED = false;

/**
 * @brief Initializes the DECODE_TABLE by reversing the ENCODE_TABLE.
 */
void initializeDecodeTable() {
    if (DECODE_TABLE_INITIALIZED) return;
    for (int i = 0; i < sizeof(ENCODE_TABLE); ++i) {
        DECODE_TABLE[ENCODE_TABLE[i]] = i;
    }
    DECODE_TABLE_INITIALIZED = true;
}

/**
 * @brief Decodes an 8-bit value to a 6-bit value using the reverse lookup table.
 * @param value The 8-bit encoded value.
 * @return The 6-bit decoded value, or -1 if the input is invalid.
 */
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

/**
 * @brief Calculates the CRC-8 checksum using the polynomial 0x31.
 * @param data Pointer to the data array.
 * @param len Length of the data.
 * @param init Initial value for the CRC calculation.
 * @return The 8-bit CRC value.
 */
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

/**
 * @brief Encodes a full RailCom datagram.
 * @details This function combines the message ID and payload, pads it to a multiple of 6 bits,
 *          splits it into 6-bit chunks, and encodes each chunk into a byte.
 * @param id The message's RailcomID.
 * @param payload The message payload.
 * @param payloadBits The number of bits in the payload.
 * @return A vector of encoded bytes.
 */
std::vector<uint8_t> encodeDatagram(RailcomID id, uint64_t payload, uint8_t payloadBits) {
    uint8_t totalBits = 4 + payloadBits;
    // The total length of the datagram (ID + payload) must be a multiple of 6.
    // If not, it needs to be padded with zeros.
    if (totalBits % 6 != 0) {
        totalBits = ((totalBits / 6) + 1) * 6;
    }
    uint8_t paddingBits = totalBits - (4 + payloadBits);
    uint8_t numBytes = totalBits / 6;

    // Combine the ID and payload into a single 64-bit integer.
    // Then shift left to add padding at the LSB (end of message).
    // Structure: [ID] [Payload] [Padding]
    uint64_t data = ((uint64_t)static_cast<uint8_t>(id) << payloadBits) | payload;
    data <<= paddingBits;

    std::vector<uint8_t> encodedBytes;
    int currentBit = totalBits - 6;
    for (int i = 0; i < numBytes; ++i) {
        // Extract the next 6-bit chunk from the combined data.
        uint8_t chunk = (data >> currentBit) & 0x3F;
        encodedBytes.push_back(encode4of8(chunk));
        currentBit -= 6;
    }
    return encodedBytes;
}

/**
 * @brief Encodes a Service Request (SRQ) message.
 * @details This is a helper function that constructs the 12-bit SRQ payload
 *          from the address and extended flag, then calls the main encodeDatagram function.
 * @param accessoryAddress The address of the accessory.
 * @param isExtended True if the address is an extended accessory address.
 * @return A vector of encoded bytes for the SRQ message.
 */
std::vector<uint8_t> encodeServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    uint16_t payload = (accessoryAddress & 0x7FF) | (isExtended ? 0x800 : 0x000);
    return encodeDatagram(RailcomID::SRQ, payload, 12);
}

}
