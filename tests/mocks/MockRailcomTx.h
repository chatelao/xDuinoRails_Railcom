#ifndef MOCK_RAILCOM_TX_H
#define MOCK_RAILCOM_TX_H

#include "RailcomTx.h"
#include <vector>

class MockRailcomTx : public RailcomTx {
public:
    struct SentMessage {
        RailcomID id;
        uint32_t data1;
        uint32_t data2;
    };

    std::vector<SentMessage> sentMessages;

    MockRailcomTx() : RailcomTx(nullptr, 0, 0) {}

    void clear() {
        sentMessages.clear();
    }

    void sendAddress(uint16_t address) override {
        sentMessages.push_back({RailcomID::ADR, address});
    }

    void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) override {
        sentMessages.push_back({RailcomID::DECODER_UNIQUE, manufacturerId, productId});
    }

    void sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps) override {
        sentMessages.push_back({RailcomID::DECODER_STATE, changeFlags, (uint32_t)((changeCount << 16) | protocolCaps)});
    }

    void sendStatus1(uint8_t status) override {
        sentMessages.push_back({RailcomID::STAT1, status});
    }
};

#endif // MOCK_RAILCOM_TX_H
