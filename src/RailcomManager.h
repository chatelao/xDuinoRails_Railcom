#ifndef RAILCOM_MANAGER_H
#define RAILCOM_MANAGER_H

#include "Railcom.h"
#include "RailcomEncoding.h"

// Enum for RailCom Message IDs as per RCN-217
enum class RailcomID {
    POM = 0,
    ADR_HIGH = 1,
    ADR_LOW = 2,
    EXT = 3,
    STAT4 = 3, // Accessory
    INFO = 4,
    STAT1 = 4, // Accessory
    TIME = 5,
    ERROR = 6,
    DYN = 7,
    XPOM_0 = 8,
    STAT2 = 8, // Accessory
    XPOM_1 = 9,
    XPOM_2 = 10,
    XPOM_3 = 11,
    CV_AUTO = 12,
    BLOCK = 13,
    RERAIL = 14
};

// Base structure for a RailCom message
struct RailcomMessage {
    RailcomID id;
};

// POM Message (ID 0)
struct PomMessage : public RailcomMessage {
    uint8_t cvValue;
};

// Address Message (ID 1 & 2)
struct AdrMessage : public RailcomMessage {
    uint16_t address;
};

// Dynamic Data Message (ID 7)
struct DynMessage : public RailcomMessage {
    uint8_t subIndex;
    uint8_t value;
};

// XPOM Message (ID 8-11)
struct XpomMessage : public RailcomMessage {
    uint8_t sequence; // 0-3, corresponds to SS in the spec
    uint8_t cvValues[4];
};

class RailcomManager {
private:
    // Parser state
    enum class ParserState { Idle, Reading };
    ParserState _parser_state;
    std::vector<uint8_t> _raw_buffer;
    uint64_t _accumulator;
    uint8_t _bits_in_accumulator;
    uint8_t _datagram_len_bits;
    RailcomID _current_id;

    // Parsed message storage
    PomMessage _pom_msg;
    AdrMessage _adr_msg;
    DynMessage _dyn_msg;
    XpomMessage _xpom_msg;


public:
    RailcomManager(RailcomSender& sender, RailcomReceiver& receiver);

    // --- Vehicle Decoder (MOB) Functions ---
    void sendPomResponse(uint8_t cvValue);
    void sendAddress(uint16_t address);
    void sendDynamicData(uint8_t subIndex, uint8_t value);
    void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]);
    void handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn);

    // --- Accessory Decoder (STAT) Functions ---
    void sendServiceRequest(uint16_t accessoryAddress, bool isExtended);
    void sendStatus1(uint8_t status);
    void sendStatus4(uint8_t status);
    void sendError(uint8_t errorCode);

    // High-level function for receiving and parsing messages
    // This will be expanded later
    RailcomMessage* readMessage();

private:
    RailcomSender& _sender;
    RailcomReceiver& _receiver;
    bool _long_address_alternator;

    // Helper to build and queue a datagram
    void sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits);
};

#endif // RAILCOM_MANAGER_H
