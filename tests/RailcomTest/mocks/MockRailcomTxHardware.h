#ifndef MOCK_RAILCOM_TX_HARDWARE_H
#define MOCK_RAILCOM_TX_HARDWARE_H

#include "RailcomTxHardware.h"
#include <vector>
#include <map>

class MockRailcomTxHardware : public RailcomTxHardware {
public:
    // --- Methods to control the mock ---
    std::vector<uint8_t> getSentBytes() { return _sentBytes; }
    void clear() {
        _sentBytes.clear();
    }

    // --- RailcomTxHardware implementation ---
    void begin() override {}
    void end() override {}
    void task() override {}

    void send_bytes(const std::vector<uint8_t>& bytes) override {
        _sentBytes.insert(_sentBytes.end(), bytes.begin(), bytes.end());
    }

private:
    std::vector<uint8_t> _sentBytes;
};

#endif // MOCK_RAILCOM_TX_HARDWARE_H
