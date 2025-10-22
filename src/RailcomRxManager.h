#ifndef RAILCOM_RX_MANAGER_H
#define RAILCOM_RX_MANAGER_H

#include "Railcom.h"
#include "RailcomRx.h"
#include "RailcomEncoding.h"

class RailcomRxManager {
public:
    RailcomRxManager(RailcomRx& receiver);

    // Main parser function. Call this repeatedly.
    // Returns a pointer to a message struct if a message has been successfully parsed.
    // Returns nullptr otherwise.
    RailcomMessage* readMessage();

private:
    RailcomRx& _receiver;

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
};

#endif // RAILCOM_RX_MANAGER_H
