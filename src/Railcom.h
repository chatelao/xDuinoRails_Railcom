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
    SRQ = 14, RERAIL = 14,

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
// A "Time" message (ID 5)
struct TimeMessage : public RailcomMessage {
    uint8_t timeValue;
    bool unit_is_second;
};
struct Stat1Message : public RailcomMessage { uint8_t status; };
struct Stat2Message : public RailcomMessage { uint8_t status; };
struct Stat4Message : public RailcomMessage { uint8_t status; };
struct ErrorMessage : public RailcomMessage { uint8_t errorCode; };
struct CvAutoMessage : public RailcomMessage { uint32_t cvAddress; uint8_t cvValue; };
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

struct RerailMessage : public RailcomMessage {
    uint8_t counter;
};

struct SrqMessage : public RailcomMessage {
    uint16_t accessoryAddress;
    bool isExtended;
};

// --- Constants ---
#define RAILCOM_ACK1 0b11110000
#define RAILCOM_ACK2 0b00001111
#define RAILCOM_NACK 0b00111100


#endif // RAILCOM_H
