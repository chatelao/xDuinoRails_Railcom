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

    void sendPomResponse(uint8_t cvValue) override {
        sentMessages.push_back({RailcomID::POM, cvValue, 0});
    }

    void sendAddress(uint16_t address) override {
        sentMessages.push_back({RailcomID::ADR_LOW, address, 0});
    }

    void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) override {
        sentMessages.push_back({RailcomID::DECODER_UNIQUE, manufacturerId, productId});
    }
};

#endif // MOCK_RAILCOM_TX_H
