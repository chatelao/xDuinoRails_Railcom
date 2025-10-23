#ifndef MOCK_RAILCOM_TX_H
#define MOCK_RAILCOM_TX_H

#include "RailcomTx.h"
#include <vector>

class MockRailcomTx : public RailcomTx {
public:
    struct SentMessage {
        RailcomID id;
        uint32_t data1;
    };

    std::vector<SentMessage> sentMessages;

    MockRailcomTx() : RailcomTx(nullptr, 0, 0) {}

    void sendStatus1(uint8_t status) override {
        sentMessages.push_back({RailcomID::STAT1, status});
    }
};

#endif // MOCK_RAILCOM_TX_H
