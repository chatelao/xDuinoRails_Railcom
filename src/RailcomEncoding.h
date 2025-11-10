/**
 * @file RailcomEncoding.h
 * @brief Handles low-level RailCom encoding, decoding, and CRC calculation.
 * @details This namespace provides a collection of utility functions for the
 *          core operations of the RailCom protocol, such as the 4-of-8 encoding
 *          scheme, CRC-8 calculation, and the construction of complete message
 *          datagrams from an ID and payload.
 */
#ifndef RAILCOM_ENCODING_H
#define RAILCOM_ENCODING_H

#include <Arduino.h>
#include <vector>
#include "Railcom.h"

/**
 * @namespace RailcomEncoding
 * @brief Provides functions for RailCom data encoding, decoding, and CRC calculation.
 */
namespace RailcomEncoding {
    /**
     * @brief Encodes a 6-bit value into an 8-bit value using the 4-of-8 encoding scheme.
     * @details This encoding ensures that there are no long sequences of 1s or 0s,
     *          which is important for reliable clock recovery in the asynchronous
     *          serial communication used by RailCom.
     * @param value The 6-bit value to encode (0-63).
     * @return The corresponding 8-bit encoded value.
     * @see RCN-217, Chapter 3.3
     */
    uint8_t encode4of8(uint8_t value);

    /**
     * @brief Decodes an 8-bit value from the 4-of-8 encoding scheme back to a 6-bit value.
     * @param value The 8-bit encoded value.
     * @return The decoded 6-bit value (0-63), or -1 if the input is not a valid 4-of-8 code.
     * @see RCN-217, Chapter 3.3
     */
    int16_t decode4of8(uint8_t value);

    /**
     * @brief Calculates the CRC-8 checksum for a block of data.
     * @details This is used for error checking in RCN-218 Data Space messages.
     * @param data A pointer to the data buffer.
     * @param len The length of the data buffer.
     * @param init The initial value for the CRC calculation (often the data space number).
     * @return The calculated 8-bit CRC value.
     * @see RCN-218, Annex B
     */
    uint8_t crc8(const uint8_t* data, size_t len, uint8_t init = 0);

    /**
     * @brief Constructs a complete, encoded RailCom message from an ID and payload.
     * @details This function performs the following steps:
     *          1. Combines the 4-bit message ID and the payload into a single bitstream.
     *          2. Splits the bitstream into 6-bit chunks.
     *          3. Encodes each 6-bit chunk using the `encode4of8` function.
     *          4. Returns the resulting vector of encoded bytes.
     * @param id The RailcomID of the message.
     * @param payload The message payload.
     * @param payloadBits The number of bits in the payload.
     * @return A vector of encoded bytes ready for transmission.
     */
    std::vector<uint8_t> encodeDatagram(RailcomID id, uint64_t payload, uint8_t payloadBits);

    /**
     * @brief Constructs a specially formatted Service Request (SRQ) message.
     * @details The SRQ message has a unique structure that doesn't fit the standard
     *          ID + payload datagram model, so it requires its own encoding function.
     * @param accessoryAddress The address of the accessory requesting service.
     * @param isExtended True if the address is an extended accessory address.
     * @return A vector of encoded bytes for the SRQ message.
     * @see RCN-217, 5.2.12
     */
    std::vector<uint8_t> encodeServiceRequest(uint16_t accessoryAddress, bool isExtended);
}

#endif // RAILCOM_ENCODING_H
