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

// Enum for RailCom Message IDs as per RCN-217
enum class RailcomID {
    POM = 0, ADR_HIGH = 1, ADR_LOW = 2, EXT = 3, STAT4 = 3,
    INFO = 4, STAT1 = 4, TIME = 5, ERROR = 6, DYN = 7,
    XPOM_0 = 8, STAT2 = 8, XPOM_1 = 9, XPOM_2 = 10, XPOM_3 = 11,
    CV_AUTO = 12, BLOCK = 13, RERAIL = 14,
    SRQ_MSG = 255 // Special ID for ID-less SRQ messages
};

// Special values for ACK/NACK
const uint8_t RAILCOM_ACK1 = 0b11110000;
const uint8_t RAILCOM_ACK2 = 0b00001111;
const uint8_t RAILCOM_NACK = 0b00111100;

// --- Message Structs ---

struct RailcomMessage { RailcomID id; };
struct PomMessage : public RailcomMessage { uint8_t cvValue; };
struct AdrMessage : public RailcomMessage { uint16_t address; };
struct DynMessage : public RailcomMessage { uint8_t subIndex; uint8_t value; };
struct XpomMessage : public RailcomMessage {
    uint8_t sequence;
    uint8_t cvValues[4];
};

#endif // RAILCOM_H
