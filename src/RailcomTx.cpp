#include "RailcomTx.h"
#include "RailcomEncoding.h"
#include "RailcomProtocolDefs.h"
#include <cstring>

RailcomTx::RailcomTx(RailcomHardware* hardware)
    : _hardware(hardware), _long_address_alternator(false) {
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

void RailcomTx::sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits) {
    _hardware->queue_message(channel, RailcomEncoding::encodeDatagram(id, payload, payloadBits));
}

void RailcomTx::sendPomResponse(uint8_t cvValue) {
    sendDatagram(2, RailcomID::POM, cvValue, 8);
}

void RailcomTx::sendAddress(uint16_t address) {
    if (address >= MIN_SHORT_ADDRESS && address <= MAX_SHORT_ADDRESS) { // Short
        if (_long_address_alternator) { // Send ADR_HIGH with 0 payload
            sendDatagram(1, RailcomID::ADR_HIGH, 0, 8);
        } else { // Send ADR_LOW with the address
            sendDatagram(1, RailcomID::ADR_LOW, address & 0x7F, 8);
        }
    } else { // Long
        if (_long_address_alternator) {
            sendDatagram(1, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
        } else {
            sendDatagram(1, RailcomID::ADR_LOW, address & 0xFF, 8);
        }
    }
    _long_address_alternator = !_long_address_alternator;
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
    uint64_t payload = ((uint64_t)manufacturerId << 32) | productId;
    payload |= ((uint64_t)RailcomID::DECODER_UNIQUE << 44);
    sendBundledDatagram(payload);
}

void RailcomTx::sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps) {
    uint64_t payload = 0;
    payload |= (uint64_t)changeFlags << 32;
    payload |= (uint64_t)changeCount << 20;
    payload |= (uint64_t)protocolCaps << 4;
    payload |= ((uint64_t)RailcomID::DECODER_STATE << 44);
    sendBundledDatagram(payload);
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

void RailcomTx::sendBundledDatagram(uint64_t payload) {
    std::vector<uint8_t> encodedBytes = RailcomEncoding::encodeBundledDatagram(payload);
    _hardware->queue_message(1, std::vector<uint8_t>(encodedBytes.begin(), encodedBytes.begin() + 2));
    _hardware->queue_message(2, std::vector<uint8_t>(encodedBytes.begin() + 2, encodedBytes.end()));
}
