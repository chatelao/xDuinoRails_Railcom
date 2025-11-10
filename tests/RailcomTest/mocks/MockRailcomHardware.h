#ifndef MOCK_RAILCOM_HARDWARE_H
#define MOCK_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include <vector>
#include <map>

class MockRailcomHardware : public RailcomHardware {
public:
    // --- Methods to control the mock ---
    void setRxBuffer(const std::vector<uint8_t>& data) {
        _rxBuffer = data;
    }
    std::vector<uint8_t> getSentBytes() { return _sentBytes; }
    void clear() {
        _rxBuffer.clear();
        _sentBytes.clear();
    }

    // --- RailcomHardware implementation ---
    void begin() override {}
    void end() override {}
    void task() override {}

    void send_bytes(const std::vector<uint8_t>& bytes) override {
        _sentBytes.insert(_sentBytes.end(), bytes.begin(), bytes.end());
    }

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
    std::vector<uint8_t> _sentBytes;
};

#endif // MOCK_RAILCOM_HARDWARE_H
