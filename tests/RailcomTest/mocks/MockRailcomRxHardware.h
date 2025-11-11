#ifndef MOCK_RAILCOM_RX_HARDWARE_H
#define MOCK_RAILCOM_RX_HARDWARE_H

#include "RailcomRxHardware.h"
#include <vector>

class MockRailcomRxHardware : public RailcomRxHardware {
public:
    // --- Methods to control the mock ---
    void setRxBuffer(const std::vector<uint8_t>& data) {
        _rxBuffer = data;
    }
    void clear() {
        _rxBuffer.clear();
    }

    // --- RailcomRxHardware implementation ---
    void begin() override {}
    void end() override {}
    void task() override {}

    int available() override {
        return _rxBuffer.size();
    }

    int read() override {
        if (_rxBuffer.empty()) {
            return -1;
        }
        int val = _rxBuffer.front();
        _rxBuffer.erase(_rxBuffer.begin());
        return val;
    }

private:
    std::vector<uint8_t> _rxBuffer;
};

#endif // MOCK_RAILCOM_RX_HARDWARE_H
