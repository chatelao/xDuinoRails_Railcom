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
        std::vector<uint8_t> data_space;
    };

    std::vector<SentMessage> sentMessages;

    MockRailcomTx() : RailcomTx(nullptr, 0, 0) {}

    void clear() {
        sentMessages.clear();
    }

    void sendPomResponse(uint8_t cvValue) override {
        sentMessages.push_back({RailcomID::POM, cvValue, 0});
    }

    void sendAddress(uint16_t address) override {
        sentMessages.push_back({RailcomID::ADR_LOW, address, 0});
    }

    void sendDynamicData(uint8_t subIndex, uint8_t value) override {
        sentMessages.push_back({RailcomID::DYN, subIndex, value});
    }

    void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]) override {
        uint16_t data = (cvValues[0] << 8) | cvValues[1];
        sentMessages.push_back({(RailcomID)((int)RailcomID::XPOM_0 + sequence), data, 0});
    }

    void sendServiceRequest(uint16_t accessoryAddress, bool isExtended) override {
        sentMessages.push_back({RailcomID::SRQ, accessoryAddress, isExtended});
    }

    void sendStatus1(uint8_t status) override {
        sentMessages.push_back({RailcomID::STAT1, status, 0});
    }

    void sendError(uint8_t errorCode) override {
        sentMessages.push_back({RailcomID::ERROR, errorCode, 0});
    }

    void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) override {
        sentMessages.push_back({RailcomID::DECODER_UNIQUE, manufacturerId, productId});
    }

    void sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum) override {
        std::vector<uint8_t> vec(data, data + len);
        sentMessages.push_back({RailcomID::INFO, dataSpaceNum, 0, vec});
    }
};

#endif // MOCK_RAILCOM_TX_H
