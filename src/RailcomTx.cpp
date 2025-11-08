#include "RailcomTx.h"
#include "RailcomEncoding.h"
#include "RailcomProtocolDefs.h"
#include <cstring>

RailcomTx::RailcomTx(RailcomHardware* hardware)
    : _hardware(hardware), _address_alternator(0), _info1_enabled(false), _info1_payload(0) {
}

void RailcomTx::begin() {
    _hardware->begin();
}

void RailcomTx::end() {
    _hardware->end();
}

void RailcomTx::task() {
    _hardware->task();
}

void RailcomTx::send_dcc_with_cutout(const DCCMessage& dccMsg) {
    _hardware->send_dcc_with_cutout(dccMsg);
}

void RailcomTx::sendDatagram(uint8_t channel, RailcomID id, uint64_t payload, uint8_t payloadBits) {
    _hardware->queue_message(channel, RailcomEncoding::encodeDatagram(id, payload, payloadBits));
}

void RailcomTx::sendPomResponse(uint8_t cvValue) {
    sendDatagram(2, RailcomID::POM, cvValue, 8);
}

void RailcomTx::sendExt(uint8_t type, uint8_t position) {
    // Per RCN-217, the type for EXT message is in the range 0-7.
    // The payload format is 00 TTTT PPPP PPPP, where TTTT must be <= 0111.
    uint16_t payload = ((type & 0x07) << 8) | position;
    sendDatagram(2, RailcomID::EXT, payload, 14);
}

void RailcomTx::sendAddress(uint16_t address) {
    uint8_t max_alternator = _info1_enabled ? 3 : 2;

    switch (_address_alternator) {
        case 0: // ADR_HIGH
            if (address >= MIN_SHORT_ADDRESS && address <= MAX_SHORT_ADDRESS) {
                sendDatagram(1, RailcomID::ADR_HIGH, 0, 8);
            } else {
                sendDatagram(1, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
            }
            break;
        case 1: // ADR_LOW
            if (address >= MIN_SHORT_ADDRESS && address <= MAX_SHORT_ADDRESS) {
                sendDatagram(1, RailcomID::ADR_LOW, address & 0x7F, 8);
            } else {
                sendDatagram(1, RailcomID::ADR_LOW, address & 0xFF, 8);
            }
            break;
        case 2: // INFO1
            if (_info1_enabled) {
                sendDatagram(1, RailcomID::INFO1, _info1_payload, 8);
            }
            break;
    }
    _address_alternator = (_address_alternator + 1) % max_alternator;
}

uint8_t RailcomTx::buildInfo1Payload(const Info1Message& info1) {
    uint8_t payload = 0;
    payload |= (info1.on_track_direction_is_positive ? 1 : 0) << 0;
    payload |= (info1.travel_direction_is_positive ? 1 : 0) << 1;
    payload |= (info1.is_moving ? 1 : 0) << 2;
    payload |= (info1.is_in_consist ? 1 : 0) << 3;
    payload |= (info1.request_addressing ? 1 : 0) << 4;
    return payload;
}

void RailcomTx::enableInfo1(const Info1Message& info1) {
    _info1_payload = buildInfo1Payload(info1);
    _info1_enabled = true;
}

void RailcomTx::disableInfo1() {
    _info1_enabled = false;
    if (_address_alternator > 1) {
        _address_alternator = 0;
    }
}

void RailcomTx::sendDynamicData(uint8_t subIndex, uint8_t value) {
    uint16_t payload = (value << 6) | (subIndex & 0x3F);
    sendDatagram(2, RailcomID::DYN, payload, 14);
}

void RailcomTx::sendCvAuto(uint32_t cvAddress, uint8_t cvValue) {
    uint32_t payload = ((cvAddress & 0xFFFFFF) << 8) | cvValue;
    sendDatagram(2, RailcomID::CV_AUTO, payload, 32);
}

void RailcomTx::sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]) {
    if (sequence > MAX_XPOM_SEQUENCE) return;
    RailcomID id = static_cast<RailcomID>(static_cast<uint8_t>(RailcomID::XPOM_0) + sequence);
    uint32_t payload = (uint32_t)cvValues[0] << 24 | (uint32_t)cvValues[1] << 16 | (uint32_t)cvValues[2] << 8 | cvValues[3];
    sendDatagram(2, id, payload, 32);
}

void RailcomTx::handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn) {
    sendDatagram(2, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
    sendDatagram(2, RailcomID::ADR_LOW, address & 0xFF, 8);
    sendDatagram(2, RailcomID::RERAIL, secondsSincePowerOn > MAX_RERAIL_SECONDS ? MAX_RERAIL_SECONDS : secondsSincePowerOn, 8);
}

void RailcomTx::sendServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    if (accessoryAddress > MAX_ACCESSORY_ADDRESS) return;
    _hardware->queue_message(1, RailcomEncoding::encodeServiceRequest(accessoryAddress, isExtended));
}

void RailcomTx::sendStatus1(uint8_t status) {
    sendDatagram(2, RailcomID::STAT1, status, 8);
}

void RailcomTx::sendStatus4(uint8_t status) {
    sendDatagram(2, RailcomID::STAT4, status, 8);
}

void RailcomTx::sendStatus2(uint8_t status) {
    sendDatagram(2, RailcomID::STAT2, status, 8);
}

void RailcomTx::sendTime(uint8_t timeValue, bool unit_is_second) {
    if (timeValue > 127) timeValue = 127;
    uint8_t payload = (unit_is_second ? 0x80 : 0x00) | timeValue;
    sendDatagram(2, RailcomID::TIME, payload, 8);
}

void RailcomTx::sendError(uint8_t errorCode) {
    sendDatagram(2, RailcomID::ERROR, errorCode, 8);
}

void RailcomTx::sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) {
    uint64_t payload = ((uint64_t)(manufacturerId & 0x0FFF) << 32) | (productId & 0xFFFFFFFF);
    sendDatagram(2, RailcomID::DECODER_UNIQUE, payload, 44);
}

void RailcomTx::sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps) {
    uint64_t payload = 0;
    payload |= (uint64_t)(changeFlags & 0xFF) << 36;
    payload |= (uint64_t)(changeCount & 0x0FFF) << 24;
    payload |= (uint64_t)(protocolCaps & 0xFFFF) << 8;
    // 8 bits reserved
    sendDatagram(2, RailcomID::DECODER_STATE, payload, 44);
}

void RailcomTx::sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum) {
    uint8_t header = (len & DATASPACE_LEN_MASK);
    uint8_t buffer[33];
    buffer[0] = header;
    memcpy(buffer + 1, data, len);
    uint8_t crc = RailcomEncoding::crc8(buffer, len + 1, dataSpaceNum);
    std::vector<uint8_t> encodedBytes;
    for (size_t i = 0; i < len + 2; ++i) {
        uint8_t chunk = (i == 0) ? header : ((i <= len) ? data[i-1] : crc);
        uint8_t encoded = RailcomEncoding::encode4of8(chunk);
        encodedBytes.push_back(encoded);
    }
    _hardware->queue_message(1, std::vector<uint8_t>(encodedBytes.begin(), encodedBytes.begin() + 2));
    _hardware->queue_message(2, std::vector<uint8_t>(encodedBytes.begin() + 2, encodedBytes.end()));
}

void RailcomTx::sendAck() {
    std::vector<uint8_t> ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2 };
    _hardware->queue_message(1, ackBytes);
    ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2 };
    _hardware->queue_message(2, ackBytes);
}

void RailcomTx::sendNack() {
    std::vector<uint8_t> nackBytes = { RAILCOM_NACK, RAILCOM_NACK };
    _hardware->queue_message(1, nackBytes);
    nackBytes = { RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK };
    _hardware->queue_message(2, nackBytes);
}

