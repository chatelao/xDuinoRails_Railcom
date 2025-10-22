#include "Railcom.h"
#include "railcom.pio.h"
#include <cstring>

// --- DCCMessage Implementation ---
DCCMessage::DCCMessage() : _len(0) {}
DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len > sizeof(_data) ? sizeof(_data) : len) {
    memcpy(_data, data, _len);
}
const uint8_t* DCCMessage::getData() const { return _data; }
size_t DCCMessage::getLength() const { return _len; }


// --- RailcomTx Implementation ---
RailcomTx* pio_sender_instance = nullptr;

void railcom_pio_irq_handler() {
    if (pio_sender_instance && pio_interrupt_get(pio_sender_instance->_pio, 0)) {
        pio_interrupt_clear(pio_sender_instance->_pio, 0);
        pio_sender_instance->_send_pending = true;
        pio_sm_set_enabled(pio_sender_instance->_pio, pio_sender_instance->_sm, false);
    }
}

RailcomTx::RailcomTx(uart_inst_t* uart, uint tx_pin, uint pio_pin)
    : _uart(uart), _tx_pin(tx_pin), _pio_pin(pio_pin), _pio(pio0), _sm(0), _offset(0), _send_pending(false), _long_address_alternator(false) {
    pio_sender_instance = this;
}

void RailcomTx::begin() {
    uart_init(_uart, 115200);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
    pio_init();
}

void RailcomTx::end() {
    uart_deinit(_uart);
    pio_sm_set_enabled(_pio, _sm, false);
    if (pio_can_remove_program(_pio, &railcom_cutout_program)) {
        pio_remove_program(_pio, &railcom_cutout_program, _offset);
    }
    pio_sm_unclaim(_pio, _sm);
}

void RailcomTx::pio_init() {
    _sm = pio_claim_unused_sm(_pio, true);
    _offset = pio_add_program(_pio, &railcom_cutout_program);
    pio_sm_config c = railcom_cutout_program_get_default_config(_offset);
    sm_config_set_out_pins(&c, _pio_pin, 1);
    sm_config_set_set_pins(&c, _pio_pin, 1);
    pio_sm_init(_pio, _sm, _offset, &c);
    pio_set_irq0_source_enabled(_pio, pis_interrupt0, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, railcom_pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

void RailcomTx::send_dcc_with_cutout(const DCCMessage& dccMsg) {
    uart_write_blocking(_uart, dccMsg.getData(), dccMsg.getLength());
    uart_tx_wait_blocking(_uart);

    gpio_set_function(_pio_pin, GPIO_FUNC_PIO0);
    pio_sm_put_blocking(_pio, _sm, 1772);
    pio_sm_set_enabled(_pio, _sm, true);
}

void RailcomTx::queue_message(uint8_t channel, const std::vector<uint8_t>& message) {
    if (channel == 1) _ch1_queue.push(message);
    else _ch2_queue.push(message);
}

void RailcomTx::task() {
    if (_send_pending) {
        _send_pending = false;

        uart_set_baudrate(_uart, 250000);
        gpio_set_function(_tx_pin, GPIO_FUNC_UART);

        send_queued_messages();
        uart_tx_wait_blocking(_uart);

        uart_set_baudrate(_uart, 115200);
    }
}

void RailcomTx::send_queued_messages() {
    if (!_ch1_queue.empty()) {
        const auto& msg = _ch1_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch1_queue.pop();
    }

    sleep_us(193);

    while (!_ch2_queue.empty()) {
        const auto& msg = _ch2_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch2_queue.pop();
    }
}

// --- RailcomRx Implementation ---
RailcomRx::RailcomRx(uart_inst_t* uart, uint rx_pin)
    : _uart(uart),
      _rx_pin(rx_pin),
      _decoder_address(0),
      _parser_state(ParserState::Idle),
      _accumulator(0),
      _bits_in_accumulator(0)
{}

void RailcomRx::begin() {
    uart_init(_uart, 250000);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RailcomRx::end() {
    uart_deinit(_uart);
}

void RailcomRx::set_decoder_address(uint16_t address) {
    _decoder_address = address;
}

bool RailcomRx::read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) {
    buffer.clear();
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (uart_is_readable(_uart)) {
            buffer.push_back(uart_getc(_uart));
        } else if (!buffer.empty()) {
            return true;
        }
    }
    return !buffer.empty();
}

// --- RailcomTxManager methods ---

void RailcomTx::sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits) {
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
    queue_message(channel, encodedBytes);
}

void RailcomTx::sendPomResponse(uint8_t cvValue) {
    sendDatagram(2, RailcomID::POM, cvValue, 8);
}

void RailcomTx::sendAddress(uint16_t address) {
    if (address >= 1 && address <= 127) { // Short
        sendDatagram(1, RailcomID::ADR_HIGH, address, 7);
    } else { // Long
        if (_long_address_alternator) {
            sendDatagram(1, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
        } else {
            sendDatagram(1, RailcomID::ADR_LOW, address & 0xFF, 8);
        }
        _long_address_alternator = !_long_address_alternator;
    }
}

void RailcomTx::sendDynamicData(uint8_t subIndex, uint8_t value) {
    uint16_t payload = (value << 6) | (subIndex & 0x3F);
    sendDatagram(2, RailcomID::DYN, payload, 14);
}

void RailcomTx::sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]) {
    if (sequence > 3) return;
    RailcomID id = static_cast<RailcomID>(static_cast<uint8_t>(RailcomID::XPOM_0) + sequence);
    uint32_t payload = (uint32_t)cvValues[0] << 24 | (uint32_t)cvValues[1] << 16 | (uint32_t)cvValues[2] << 8  | cvValues[3];
    sendDatagram(2, id, payload, 32);
}

void RailcomTx::handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn) {
    sendDatagram(2, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
    sendDatagram(2, RailcomID::ADR_LOW, address & 0xFF, 8);
    sendDatagram(2, RailcomID::RERAIL, secondsSincePowerOn > 255 ? 255 : secondsSincePowerOn, 8);
}

void RailcomTx::sendServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    if (accessoryAddress > 2047) return;
    uint16_t payload = (accessoryAddress & 0x7FF) | (isExtended ? 0x800 : 0x000);
    std::vector<uint8_t> encodedBytes;
    encodedBytes.push_back(RailcomEncoding::encode4of8((payload >> 6) & 0x3F));
    encodedBytes.push_back(RailcomEncoding::encode4of8(payload & 0x3F));
    queue_message(1, encodedBytes);
}

void RailcomTx::sendStatus1(uint8_t status) {
    sendDatagram(2, RailcomID::STAT1, status, 8);
}

void RailcomTx::sendStatus4(uint8_t status) {
    sendDatagram(2, RailcomID::STAT4, status, 8);
}

void RailcomTx::sendError(uint8_t errorCode) {
    sendDatagram(2, RailcomID::ERROR, errorCode, 8);
}

// --- RCN-218 DCC-A Functions ---

void RailcomTx::sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) {
    uint64_t payload = ((uint64_t)manufacturerId << 32) | productId;
    payload |= ((uint64_t)RailcomID::DECODER_UNIQUE << 44);
    sendBundledDatagram(payload);
}

void RailcomTx::sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps) {
    uint8_t data[5];
    uint64_t payload = 0;

    // EXT1, EXT2
    payload |= (uint64_t)changeFlags << 32;

    // EXT3, DATA[31..24]
    payload |= (uint64_t)changeCount << 20;

    // DATA[23..8]
    payload |= (uint64_t)protocolCaps << 4;

    // Build byte array for CRC
    data[0] = (uint8_t)RailcomID::DECODER_STATE << 4 | (payload >> 40);
    data[1] = (payload >> 32);
    data[2] = (payload >> 24);
    data[3] = (payload >> 16);
    data[4] = (payload >> 8);

    uint8_t crc = RailcomEncoding::crc8(data, 5);
    payload |= crc;

    payload |= ((uint64_t)RailcomID::DECODER_STATE << 44);
    sendBundledDatagram(payload);
}

void RailcomTx::sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum) {
    uint8_t header = (len & 0x1F);
    uint8_t buffer[33];
    buffer[0] = header;
    memcpy(buffer + 1, data, len);
    uint8_t crc = RailcomEncoding::crc8(buffer, len + 1, dataSpaceNum);

    std::vector<uint8_t> encodedBytes;
    int currentBit = (len + 1) * 8 - 8;
    for (size_t i = 0; i < len + 2; ++i) {
        uint8_t chunk = (i == 0) ? header : ((i <= len) ? data[i-1] : crc);
        uint8_t encoded = RailcomEncoding::encode4of8(chunk);
        encodedBytes.push_back(encoded);
    }
    queue_message(1, std::vector<uint8_t>(encodedBytes.begin(), encodedBytes.begin() + 2));
    queue_message(2, std::vector<uint8_t>(encodedBytes.begin() + 2, encodedBytes.end()));
}

void RailcomTx::sendAck() {
    std::vector<uint8_t> ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2 };
    queue_message(1, ackBytes);
    ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2 };
    queue_message(2, ackBytes);
}

void RailcomTx::sendNack() {
    std::vector<uint8_t> nackBytes = { RAILCOM_NACK, RAILCOM_NACK };
    queue_message(1, nackBytes);
    nackBytes = { RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK };
    queue_message(2, nackBytes);
}

void RailcomTx::sendBundledDatagram(uint64_t payload) {
    std::vector<uint8_t> encodedBytes;
    int currentBit = 42;
    for (int i = 0; i < 8; ++i) {
        uint8_t chunk = (payload >> currentBit) & 0x3F;
        encodedBytes.push_back(RailcomEncoding::encode4of8(chunk));
        currentBit -= 6;
    }
    queue_message(1, std::vector<uint8_t>(encodedBytes.begin(), encodedBytes.begin() + 2));
    queue_message(2, std::vector<uint8_t>(encodedBytes.begin() + 2, encodedBytes.end()));
}

// --- RailcomRxManager methods ---
uint8_t RailcomRx::get_datagram_payload_bits(RailcomID id) {
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

RailcomMessage* RailcomRx::readMessage() {
    std::vector<uint8_t> new_bytes;
    if (read_raw_bytes(new_bytes, 5)) {
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
