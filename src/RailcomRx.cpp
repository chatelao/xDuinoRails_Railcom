#include "RailcomRx.h"

// Helper function to determine payload size from Railcom ID
static uint8_t get_datagram_payload_bits(RailcomID id) {
    switch (id) {
        case RailcomID::POM: case RailcomID::STAT4: case RailcomID::STAT1:
        case RailcomID::TIME: case RailcomID::ERROR: case RailcomID::RERAIL:
            return 8;
        case RailcomID::ADR_HIGH: return 7;
        case RailcomID::ADR_LOW: return 8;
        case RailcomID::DYN: return 14;
        case RailcomID::XPOM_0: case RailcomID::XPOM_1: case RailcomID::XPOM_2:
        case RailcomID::XPOM_3: case RailcomID::CV_AUTO:
            return 32;
        default: return 0;
    }
}

RailcomRx::RailcomRx(uart_inst_t* uart, uint rx_pin)
    : _uart(uart),
      _rx_pin(rx_pin),
      _decoder_address(0),
      _parser_state(ParserState::Idle),
      _accumulator(0),
      _bits_in_accumulator(0)
{}

void RailcomRx::set_decoder_address(uint16_t address) {
    _decoder_address = address;
}

void RailcomRx::begin() {
    uart_init(_uart, 250000); // Railcom baud rate
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
    uart_set_hw_flow(_uart, false, false);
    uart_set_format(_uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(_uart, true);
}

void RailcomRx::end() {
    uart_deinit(_uart);
}

bool RailcomRx::read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) {
    buffer.clear();
    uint64_t start_time = time_us_64();
    while ((time_us_64() - start_time) < (timeout_ms * 1000)) {
        if (uart_is_readable(_uart)) {
            uint8_t ch = uart_getc(_uart);
            buffer.push_back(ch);
        }
    }
    return !buffer.empty();
}

RailcomMessage* RailcomRx::readMessage() {
    std::vector<uint8_t> new_bytes;
    if (read_raw_bytes(new_bytes, 5)) {
        for (uint8_t byte : new_bytes) {
            _raw_buffer.push_back(byte);
        }
    }

    while (!_raw_buffer.empty()) {
        int16_t chunk = Railcom::decode4of8(_raw_buffer.front());
        _raw_buffer.erase(_raw_buffer.begin());

        if (chunk < 0) {
            _parser_state = ParserState::Idle;
            _bits_in_accumulator = 0;
            continue;
        }

        _accumulator = (_accumulator << 6) | chunk;
        _bits_in_accumulator += 6;

        if (_parser_state == ParserState::Idle && _bits_in_accumulator >= 8) {
            uint8_t id_val = (_accumulator >> (_bits_in_accumulator - 4)) & 0x0F;
            _current_id = static_cast<RailcomID>(id_val);
            _datagram_len_bits = 4 + get_datagram_payload_bits(_current_id);
            if (_datagram_len_bits > 4) {
                 _parser_state = ParserState::Reading;
            } else {
                 _parser_state = ParserState::Idle;
                 _bits_in_accumulator = 0;
            }
        }

        if (_parser_state == ParserState::Reading && _bits_in_accumulator >= _datagram_len_bits) {
            _parser_state = ParserState::Idle;
            uint64_t mask = (1ULL << (_datagram_len_bits - 4)) - 1;
            uint32_t payload = (_accumulator >> (_bits_in_accumulator - _datagram_len_bits)) & mask;
            _bits_in_accumulator = 0;

            switch (_current_id) {
                case RailcomID::POM:
                    _pom_msg = {RailcomID::POM, (uint8_t)payload};
                    return &_pom_msg;
                case RailcomID::ADR_HIGH:
                case RailcomID::ADR_LOW:
                    _adr_msg = {_current_id, (uint16_t)payload};
                    return &_adr_msg;
                case RailcomID::DYN:
                    _dyn_msg = {RailcomID::DYN, (uint8_t)(payload & 0x3F), (uint8_t)(payload >> 6)};
                    return &_dyn_msg;
                case RailcomID::XPOM_0:
                case RailcomID::XPOM_1:
                case RailcomID::XPOM_2:
                case RailcomID::XPOM_3:
                    _xpom_msg.id = _current_id;
                    _xpom_msg.sequence = static_cast<uint8_t>(_current_id) - static_cast<uint8_t>(RailcomID::XPOM_0);
                    _xpom_msg.cvValues[0] = (payload >> 24) & 0xFF;
                    _xpom_msg.cvValues[1] = (payload >> 16) & 0xFF;
                    _xpom_msg.cvValues[2] = (payload >> 8) & 0xFF;
                    _xpom_msg.cvValues[3] = payload & 0xFF;
                    return &_xpom_msg;
                default:
                    break;
            }
        }
    }
    return nullptr;
}
