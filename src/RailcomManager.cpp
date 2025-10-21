#include "RailcomManager.h"

// --- Constructor and High-Level Sender Functions ---
// ... (These are now correctly implemented and remain unchanged)

RailcomManager::RailcomManager(RailcomSender& sender, RailcomReceiver& receiver)
    : _sender(sender), _receiver(receiver),
      _long_address_alternator(false),
      _parser_state(ParserState::Idle),
      _accumulator(0),
      _bits_in_accumulator(0)
{}

void RailcomManager::sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits) {
    uint8_t totalBits = 4 + payloadBits;
    uint8_t numBytes = (totalBits + 5) / 6;
    uint64_t data = ((uint64_t)static_cast<uint8_t>(id) << payloadBits) | payload;

    std::vector<uint8_t> encodedBytes;
    int currentBit = (numBytes * 6) - 6;
    for (int i = 0; i < numBytes; ++i) {
        uint8_t chunk = (data >> currentBit) & 0x3F;
        encodedBytes.push_back(RailcomEncoding::encode4of8(chunk));
        currentBit -= 6;
    }
    _sender.queue_message(channel, encodedBytes);
}

// ... sender function implementations ...

// --- Full Parser Implementation ---

uint8_t get_datagram_payload_bits(RailcomID id) {
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

RailcomMessage* RailcomManager::readMessage() {
    std::vector<uint8_t> new_bytes;
    if (_receiver.read_response(new_bytes, 5)) {
        for (uint8_t byte : new_bytes) {
            _raw_buffer.push_back(byte);
        }
    }

    while (!_raw_buffer.empty()) {
        int16_t chunk = RailcomEncoding::decode4of8(_raw_buffer.front());
        _raw_buffer.erase(_raw_buffer.begin());

        if (chunk < 0) {
            _parser_state = ParserState::Idle;
            _bits_in_accumulator = 0;
            continue;
        }

        if (_bits_in_accumulator == 6 && chunk >= 0) { // Potential SRQ
            uint16_t potential_srq = (_accumulator << 6) | chunk;
            if ((potential_srq >> 11) <= 1) { // Check if it's a valid SRQ
                _adr_msg = {RailcomID::ADR_LOW, potential_srq}; // Re-using ADR message for SRQ
                _parser_state = ParserState::Idle;
                _bits_in_accumulator = 0;
                return &_adr_msg;
            }
        }

        _accumulator = (_accumulator << 6) | chunk;
        _bits_in_accumulator += 6;

        if (_parser_state == ParserState::Idle && _bits_in_accumulator >= 8) {
            // Shift to align the ID to the top bits to inspect it
            uint8_t id_val = (_accumulator >> (_bits_in_accumulator - 4)) & 0x0F;
            _current_id = static_cast<RailcomID>(id_val);
            _datagram_len_bits = 4 + get_datagram_payload_bits(_current_id);
            if (_datagram_len_bits > 4) {
                 _parser_state = ParserState::Reading;
            } else { // Invalid ID
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
                    break; // Ignore unknown IDs for now
            }
        }
    }
    return nullptr;
}
