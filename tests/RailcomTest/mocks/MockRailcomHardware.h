#ifndef MOCK_RAILCOM_HARDWARE_H
#define MOCK_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include <vector>

class MockRailcomHardware : public RailcomHardware {
public:
    // --- Methods to control the mock ---
    void setRxBuffer(const std::vector<uint8_t>& data) {
        _rxBuffer = data;
    }
    std::vector<uint8_t> getQueuedMessages() { return _queuedMessages; }
    void clear() {
        _rxBuffer.clear();
        _queuedMessages.clear();
    }

    // --- RailcomHardware implementation ---
    void begin() override {}
    void end() override {}
    void task() override {}

    void send_dcc_with_cutout(const DCCMessage& dccMsg) override {
        // For now, do nothing.
    }

    void queue_message(uint8_t channel, const std::vector<uint8_t>& message) override {
        _queuedMessages.insert(_queuedMessages.end(), message.begin(), message.end());
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
    std::vector<uint8_t> _queuedMessages;
};

#endif // MOCK_RAILCOM_HARDWARE_H
