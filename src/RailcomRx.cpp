#include "RailcomRx.h"
#include "RailcomEncoding.h"
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
    uint64_t decodedData = 0;
    int bitCount = 0;
    for (uint8_t byte : buffer) {
        int16_t decodedChunk = RailcomEncoding::decode4of8(byte);
        if (decodedChunk == -1) return nullptr; // Invalid encoding
        decodedData = (decodedData << 6) | decodedChunk;
        bitCount += 6;
    }

    if (bitCount < 4) return nullptr;

    RailcomID id = static_cast<RailcomID>((decodedData >> (bitCount - 4)) & 0x0F);
    uint64_t payload = decodedData & ((1ULL << (bitCount - 4)) - 1);

    switch (id) {
        case RailcomID::POM: {
            PomMessage* msg = new PomMessage();
            msg->id = id;
            msg->cvValue = payload;
            return msg;
        }
        case RailcomID::ADR_HIGH: {
            AdrMessage* msg = new AdrMessage();
            msg->id = id;
            msg->address = payload >> 1; // 7-bit address is padded
            return msg;
        }
        case RailcomID::TIME: {
            TimeMessage* msg = new TimeMessage();
            msg->id = id;
            msg->resolution = (payload >> 7) & 0x01;
            msg->time = payload & 0x7F;
            return msg;
        }
        case RailcomID::STAT2: {
            Stat2Message* msg = new Stat2Message();
            msg->id = id;
            msg->status = payload;
            return msg;
        }
        case RailcomID::CV_AUTO: {
            CvAutoMessage* msg = new CvAutoMessage();
            msg->id = id;
            msg->cvAddress = (payload >> 8) & 0xFFFFFF;
            msg->cvValue = payload & 0xFF;
            return msg;
        }
        default:
            return nullptr;
    }
}
