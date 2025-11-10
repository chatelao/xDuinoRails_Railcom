#ifndef MOCK_RAILCOM_HARDWARE_H
#define MOCK_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include <vector>
#include <map>

class MockRailcomHardware : public RailcomHardware {
public:
    // --- Methods to control the mock ---
    void setRxBuffer(const std::map<uint8_t, std::vector<uint8_t>>& data) {
        if (data.count(1)) _rxBuffer = data.at(1);
        if (data.count(2)) {
            _rxBuffer.insert(_rxBuffer.end(), data.at(2).begin(), data.at(2).end());
        }
    }
    std::map<uint8_t, std::vector<uint8_t>> getQueuedMessages() { return _queuedMessages; }
    void clear() {
        _rxBuffer.clear();
        _queuedMessages.clear();
    }

    // --- RailcomHardware implementation ---
    void begin() override {}
    void end() override {}
    void task() override {}

    void on_cutout_start() override {
        // For now, do nothing. In a real test, this might set a flag.
    }

    void queue_message(uint8_t channel, const std::vector<uint8_t>& message) override {
        _queuedMessages[channel].insert(_queuedMessages[channel].end(), message.begin(), message.end());
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
    std::map<uint8_t, std::vector<uint8_t>> _queuedMessages;
};

#endif // MOCK_RAILCOM_HARDWARE_H
