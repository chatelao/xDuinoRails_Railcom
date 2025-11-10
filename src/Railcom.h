/**
 * @file Railcom.h
 * @brief Common data structures, enums, and constants for the RailCom library.
 * @details This file defines the core data types used throughout the library,
 *          including message structures for both RCN-217 and RCN-218 specifications.
 */
#ifndef RAILCOM_H
#define RAILCOM_H

#include <Arduino.h>

// --- Common Data Structures ---

/**
 * @class DCCMessage
 * @brief Represents a single DCC (Digital Command Control) packet.
 * @details This class encapsulates the raw data and length of a DCC message,
 *          providing a standardized way to handle DCC packets within the library.
 */
class DCCMessage {
public:
    /**
     * @brief Default constructor. Creates an empty DCC message.
     */
    DCCMessage();
    /**
     * @brief Constructs a DCCMessage from raw data.
     * @param data A pointer to the byte array containing the DCC packet data.
     * @param len The length of the data array.
     */
    DCCMessage(const uint8_t* data, size_t len);
    /**
     * @brief Gets a pointer to the raw DCC message data.
     * @return A const pointer to the internal data buffer.
     */
    const uint8_t* getData() const;
    /**
     * @brief Gets the length of the DCC message.
     * @return The length of the message in bytes.
     */
    size_t getLength() const;
private:
    uint8_t _data[12]; ///< Internal buffer to store the DCC packet data.
    size_t _len;       ///< The length of the stored DCC packet.
};

/**
 * @enum RailcomID
 * @brief Defines the Message IDs for RailCom communication.
 * @details The values are based on the RCN-217 and RCN-218 specifications.
 *          Note that some IDs are shared between different message types and
 *          must be disambiguated by context (e.g., decoder type) or message length.
 * @see RCN-217, Chapter 5
 * @see RCN-218, Chapter 4
 */
enum class RailcomID {
    // RCN-217
    POM = 0,         ///< Program on Main, response to a POM command.
    ADR_HIGH = 1,    ///< Higher 6 bits of a 14-bit address.
    ADR_LOW = 2,     ///< Lower 8 bits of a 14-bit address.
    EXT = 3,         ///< Extended Message (Mobile Decoder).
    STAT4 = 3,       ///< Status Message for 4 turnouts (Stationary Decoder).
    INFO1 = 3,       ///< Basic Information (Mobile Decoder).
    INFO = 4,        ///< Driving Information (Mobile Decoder).
    STAT1 = 4,       ///< Status Message for 1 turnout (Stationary Decoder).
    TIME = 5,        ///< Time Message.
    ERROR = 6,       ///< Error Message.
    DYN = 7,         ///< Dynamic Data.
    XPOM_0 = 8,      ///< Extended POM, part 0.
    STAT2 = 8,       ///< Status Message for 2 turnouts (Stationary Decoder).
    XPOM_1 = 9,      ///< Extended POM, part 1.
    XPOM_2 = 10,     ///< Extended POM, part 2.
    XPOM_3 = 11,     ///< Extended POM, part 3.
    CV_AUTO = 12,    ///< Automatic CV Broadcast.
    SRQ = 14,        ///< Service Request (Accessory Decoder).
    RERAIL = 14,     ///< Rerailing Message (Mobile Decoder).

    // RCN-218
    BLOCK = 13,          ///< Block Message (for Logon).
    DECODER_STATE = 13,  ///< Decoder State (for Logon).
    DECODER_UNIQUE = 15  ///< Decoder Unique ID (for Logon).
};

/**
 * @namespace RCN218
 * @brief Defines constants related to RCN-218 (RailComPlus) DCC-A commands.
 * @details These constants represent the specific command bytes used in the
 *          RailComPlus automatic logon procedure.
 * @see RCN-218, Chapter 3
 */
namespace RCN218 {
    const uint8_t DCC_A_ADDRESS = 254;          ///< The special broadcast address for DCC-A commands.
    const uint8_t CMD_LOGON_ENABLE = 0xF0;      ///< Base command for enabling logon (GG bits to be added).
    const uint8_t CMD_SELECT = 0xD0;            ///< Base command for selecting a decoder (HHHH bits to be added).
    const uint8_t CMD_LOGON_ASSIGN = 0xE0;      ///< Base command for assigning a slot (HHHH bits to be added).
    const uint8_t CMD_GET_DATA_START = 0x00;    ///< Command to start reading data from a decoder.
    const uint8_t CMD_GET_DATA_CONT = 0x01;     ///< Command to continue reading data from a decoder.
    const uint8_t CMD_SET_DATA = 0x02;          ///< Command to write data to a decoder.
    const uint8_t CMD_SET_DATA_END = 0x03;      ///< Command to finalize writing data to a decoder.
}

/**
 * @enum DecoderContext
 * @brief Used to disambiguate RailCom messages that share the same ID.
 * @details The context (e.g., whether the decoder is mobile or stationary)
 *          is required to correctly parse messages with overlapping IDs
 *          like ID 3 (EXT, STAT4, INFO1) and ID 4 (INFO, STAT1).
 */
enum class DecoderContext {
    UNKNOWN,    ///< The context is not yet known.
    MOBILE,     ///< The decoder is a mobile decoder (e.g., in a locomotive).
    STATIONARY  ///< The decoder is a stationary decoder (e.g., for turnouts).
};

// --- Message Structs ---

/** @brief Base struct for all RailCom messages, containing the message ID. */
struct RailcomMessage { RailcomID id; };

/** @brief A POM (Program on Main) message (ID 0). @see RCN-217, 5.2.1 */
struct PomMessage : public RailcomMessage { uint8_t cvValue; };

/** @brief An address message (ID 1 or 2). @see RCN-217, 5.2.2 & 5.2.3 */
struct AdrMessage : public RailcomMessage { uint16_t address; };

/** @brief A Dynamic Data message (ID 7). @see RCN-217, 5.2.8 */
struct DynMessage : public RailcomMessage { uint8_t subIndex; uint8_t value; };

/** @brief An Extended Message (ID 3, Mobile). @see RCN-217, 5.2.4 */
struct ExtMessage : public RailcomMessage { uint8_t type; uint8_t position; };

/** @brief A Basic Information message (ID 3, Mobile). @see RCN-217, 5.2.4 */
struct Info1Message : public RailcomMessage {
    bool on_track_direction_is_positive;
    bool travel_direction_is_positive;
    bool is_moving;
    bool is_in_consist;
    bool request_addressing;
};

/** @brief A Driving Information message (ID 4, Mobile). @see RCN-217, 5.2.5 */
struct InfoMessage : public RailcomMessage {
    uint16_t speed;
    uint8_t motorLoad;
    uint8_t statusFlags;
};

/** @brief A Time message (ID 5). @see RCN-217, 5.2.6 */
struct TimeMessage : public RailcomMessage {
    uint8_t timeValue;
    bool unit_is_second;
};

/** @brief A Status message for 1 turnout (ID 4, Stationary). @see RCN-217, 5.2.5 */
struct Stat1Message : public RailcomMessage { uint8_t status; };

/** @brief A Status message for 2 turnouts (ID 8, Stationary). @see RCN-217, 5.2.9 */
struct Stat2Message : public RailcomMessage { uint8_t status; };

/** @brief A Status message for 4 turnouts (ID 3, Stationary). @see RCN-217, 5.2.4 */
struct Stat4Message : public RailcomMessage { uint8_t status; };

/** @brief An Error message (ID 6). @see RCN-217, 5.2.7 */
struct ErrorMessage : public RailcomMessage { uint8_t errorCode; };

/** @brief An Automatic CV Broadcast message (ID 12). @see RCN-217, 5.2.11 */
struct CvAutoMessage : public RailcomMessage { uint32_t cvAddress; uint8_t cvValue; };

/** @brief An Extended POM message (IDs 8-11). @see RCN-217, 5.2.9 */
struct XpomMessage : public RailcomMessage {
    uint8_t sequence;
    uint8_t cvValues[4];
};

/** @brief A Decoder Unique ID message (ID 15). @see RCN-218, 4.3 */
struct DecoderUniqueMessage : public RailcomMessage {
    uint16_t manufacturerId;
    uint32_t productId;
};

/** @brief A Decoder State message (ID 13). @see RCN-218, 4.2 */
struct DecoderStateMessage : public RailcomMessage {
    uint8_t changeFlags;
    uint16_t changeCount;
    uint16_t protocolCaps;
};

/** @brief A Rerailing message (ID 14, Mobile). @see RCN-217, 5.2.12 */
struct RerailMessage : public RailcomMessage {
    uint8_t counter;
};

/** @brief A Service Request message (ID 14, Accessory). @see RCN-217, 5.2.12 */
struct SrqMessage : public RailcomMessage {
    uint16_t accessoryAddress;
    bool isExtended;
};

/** @brief A Block message (ID 13). @see RCN-218, 4.1 */
struct BlockMessage : public RailcomMessage {
    uint32_t data;
};

/** @brief Maximum payload size for a Data Space message. */
#define MAX_DATA_SPACE_PAYLOAD 16
/** @brief A Data Space message (No ID, special format). @see RCN-218, 5.2 */
struct DataSpaceMessage : public RailcomMessage {
    uint8_t len;
    uint8_t data[MAX_DATA_SPACE_PAYLOAD];
    uint8_t crc;
    uint8_t dataSpaceNum; ///< From the DCC command, not the RailCom message itself.
    bool crc_ok;          ///< True if the received CRC matches the calculated one.
};

// --- Constants ---
/** @brief First part of the RailCom ACK signal. @see RCN-217, 3.4.1 */
#define RAILCOM_ACK1 0b11110000
/** @brief Second part of the RailCom ACK signal. @see RCN-217, 3.4.1 */
#define RAILCOM_ACK2 0b00001111
/** @brief The RailCom NACK (Not Acknowledge) signal. @see RCN-218, 5.2 */
#define RAILCOM_NACK 0b00111100


#endif // RAILCOM_H
