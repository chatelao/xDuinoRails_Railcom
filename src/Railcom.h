#ifndef RAILCOM_H
#define RAILCOM_H

#include <Arduino.h>

// --- Common Data Structures ---

class DCCMessage {
public:
    DCCMessage();
    DCCMessage(const uint8_t* data, size_t len);
    const uint8_t* getData() const;
    size_t getLength() const;
private:
    uint8_t _data[12];
    size_t _len;
};

// Enum for RailCom Message IDs as per RCN-217 and RCN-218
enum class RailcomID {
    // RCN-217
    POM = 0, ADR_HIGH = 1, ADR_LOW = 2, EXT = 3, STAT4 = 3,
    INFO = 4, STAT1 = 4, TIME = 5, ERROR = 6, DYN = 7,
    XPOM_0 = 8, STAT2 = 8, XPOM_1 = 9, XPOM_2 = 10, XPOM_3 = 11,
    CV_AUTO = 12,
    RERAIL = 14,

    // RCN-218
    DECODER_STATE = 13,
    DECODER_UNIQUE = 15
};

// Constants for RCN-218 DCC-A Commands
namespace RCN218 {
    const uint8_t DCC_A_ADDRESS = 254;
    const uint8_t CMD_LOGON_ENABLE = 0xF0; // Base, GG bits will be added
    const uint8_t CMD_SELECT = 0xD0;       // Base, HHHH bits will be added
    const uint8_t CMD_LOGON_ASSIGN = 0xE0; // Base, HHHH bits will be added
    const uint8_t CMD_GET_DATA_START = 0x00;
    const uint8_t CMD_GET_DATA_CONT = 0x01;
    const uint8_t CMD_SET_DATA = 0x02;
    const uint8_t CMD_SET_DATA_END = 0x03;
}

// --- Message Structs ---

struct RailcomMessage { RailcomID id; };
struct PomMessage : public RailcomMessage { uint8_t cvValue; };
struct AdrMessage : public RailcomMessage { uint16_t address; };
struct DynMessage : public RailcomMessage { uint8_t subIndex; uint8_t value; };
struct XpomMessage : public RailcomMessage {
    uint8_t sequence;
    uint8_t cvValues[4];
};

struct DecoderUniqueMessage : public RailcomMessage {
    uint16_t manufacturerId;
    uint32_t productId;
};

struct DecoderStateMessage : public RailcomMessage {
    uint8_t changeFlags;
    uint16_t changeCount;
    uint16_t protocolCaps;
};

// --- Encoding/Decoding ---

namespace Railcom {

    // Special values for ACK/NACK, as defined in the RailCom specification.
    const uint8_t ACK1 = 0b11110000;
    const uint8_t ACK2 = 0b00001111;
    const uint8_t NACK = 0b00111100;

    /**
     * @brief Encodes 6 bits of data into a 4-out-of-8 encoded byte.
     *
     * This encoding scheme is defined in RCN-217, Table 2. It ensures that
     * every transmitted byte has exactly four '1' bits and four '0' bits,
     * which helps with clock recovery and error detection on the receiver side.
     *
     * @param value The 6-bit value (0-63) to encode.
     * @return The 8-bit encoded byte. Returns 0 if the input value is > 63.
     */
    uint8_t encode4of8(uint8_t value);

    /**
     * @brief Decodes a 4-out-of-8 encoded byte back into 6 bits of data.
     *
     * It also checks for the special ACK and NACK values. The function verifies
     * that the byte has a Hamming weight of 4 (four '1' bits).
     *
     * @param encodedByte The 8-bit value to decode.
     * @return The decoded 6-bit value (0-63).
     *         Returns 0x100 for ACK, 0x101 for NACK, or -1 for an invalid byte.
     */
    int16_t decode4of8(uint8_t encodedByte);

    /**
     * @brief Calculates the RCN-218 CRC8 checksum.
     *
     * This is a standard CRC-8 calculation with a specific polynomial (0x31, reversed 0x8C)
     * as defined in the RailCom specification. It's used to validate the integrity
     * of multi-byte data packets.
     *
     * @param data Pointer to the data array.
     * @param len  Number of bytes in the array.
     * @param init Initial CRC value (defaults to 0).
     * @return The calculated 8-bit CRC value.
     */
    uint8_t crc8(const uint8_t* data, size_t len, uint8_t init = 0);

} // namespace Railcom

#endif // RAILCOM_H
