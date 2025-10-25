#include "RailcomRx.h"
#include <cstring>
#include "pico/stdlib.h"

RailcomRx::RailcomRx(RailcomHardware* hardware)
    : _hardware(hardware) {}

void RailcomRx::begin() {
    _hardware->begin();
}

void RailcomRx::task() {
    // task() is not used in the restored logic,
    // so this is intentionally left empty.
}

bool RailcomRx::read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) {
    buffer.clear();
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (_hardware->available()) {
            buffer.push_back(_hardware->read());
        } else if (!buffer.empty()) {
            return true;
        }
    }
    return !buffer.empty();
}

RailcomMessage* RailcomRx::readMessage() {
    std::vector<uint8_t> buffer;
    if (read_raw_bytes(buffer, 50)) {
        return parseMessage(buffer);
    }
    return nullptr;
}

RailcomMessage* RailcomRx::parseMessage(const std::vector<uint8_t>& buffer) {
    // This is a placeholder implementation. A real implementation would
    // need to decode the 4-of-8 encoding and parse the message format.
    if (buffer.size() >= 2) {
        PomMessage* msg = new PomMessage();
        msg->id = RailcomID::POM;
        msg->cvValue = buffer[1];
        return msg;
    }
    return nullptr;
}
