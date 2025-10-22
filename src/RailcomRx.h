#ifndef RAILCOM_RX_H
#define RAILCOM_RX_H

#include <Arduino.h>
#include <vector>
#include "hardware/uart.h"
#include "Railcom.h"
#include "RailcomEncoding.h"

class RailcomRx {
public:
    RailcomRx(uart_inst_t* uart, uint rx_pin);
    void begin();
    void end();
    void set_decoder_address(uint16_t address);
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);

    // Main parser function. Call this repeatedly.
    // Returns a pointer to a message struct if a message has been successfully parsed.
    // Returns nullptr otherwise.
    RailcomMessage* readMessage();

private:
    uart_inst_t* _uart;
    uint _rx_pin;
    uint16_t _decoder_address;

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

    uint8_t get_datagram_payload_bits(RailcomID id);
};

#endif // RAILCOM_RX_H
