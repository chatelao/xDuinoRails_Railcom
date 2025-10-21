#include "RailcomTxManager.h"

RailcomTxManager::RailcomTxManager(RailcomSender& sender)
    : _sender(sender), _long_address_alternator(false) {}

void RailcomTxManager::sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits) {
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

void RailcomTxManager::sendPomResponse(uint8_t cvValue) {
    sendDatagram(2, RailcomID::POM, cvValue, 8);
}

void RailcomTxManager::sendAddress(uint16_t address) {
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

void RailcomTxManager::sendDynamicData(uint8_t subIndex, uint8_t value) {
    uint16_t payload = (value << 6) | (subIndex & 0x3F);
    sendDatagram(2, RailcomID::DYN, payload, 14);
}

void RailcomTxManager::sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]) {
    if (sequence > 3) return;
    RailcomID id = static_cast<RailcomID>(static_cast<uint8_t>(RailcomID::XPOM_0) + sequence);
    uint32_t payload = (uint32_t)cvValues[0] << 24 | (uint32_t)cvValues[1] << 16 | (uint32_t)cvValues[2] << 8  | cvValues[3];
    sendDatagram(2, id, payload, 32);
}

void RailcomTxManager::handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn) {
    sendDatagram(2, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
    sendDatagram(2, RailcomID::ADR_LOW, address & 0xFF, 8);
    sendDatagram(2, RailcomID::RERAIL, secondsSincePowerOn > 255 ? 255 : secondsSincePowerOn, 8);
}

void RailcomTxManager::sendServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    if (accessoryAddress > 2047) return;
    uint16_t payload = (accessoryAddress & 0x7FF) | (isExtended ? 0x800 : 0x000);
    std::vector<uint8_t> encodedBytes;
    encodedBytes.push_back(RailcomEncoding::encode4of8((payload >> 6) & 0x3F));
    encodedBytes.push_back(RailcomEncoding::encode4of8(payload & 0x3F));
    _sender.queue_message(1, encodedBytes);
}

void RailcomTxManager::sendStatus1(uint8_t status) {
    sendDatagram(2, RailcomID::STAT1, status, 8);
}

void RailcomTxManager::sendStatus4(uint8_t status) {
    sendDatagram(2, RailcomID::STAT4, status, 8);
}

void RailcomTxManager::sendError(uint8_t errorCode) {
    sendDatagram(2, RailcomID::ERROR, errorCode, 8);
}
