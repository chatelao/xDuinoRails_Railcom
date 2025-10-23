#ifndef MOCK_RAILCOM_TX_H
#define MOCK_RAILCOM_TX_H

#include "RailcomTx.h"

class MockRailcomTx : public RailcomTx {
public:
    MockRailcomTx() : RailcomTx(nullptr, 0, 0) {}

    void queue_message(uint8_t channel, const std::vector<uint8_t>& data) {
        last_channel = channel;
        last_data = data;
    }

    uint8_t last_channel;
    std::vector<uint8_t> last_data;
};

#endif // MOCK_RAILCOM_TX_H
