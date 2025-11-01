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
            // Per RCN-217 for long addresses, the address is in the lower 6 bits.
            msg->address = payload & 0x3F;
            return msg;
        }
        case RailcomID::ADR_LOW: {
            AdrMessage* msg = new AdrMessage();
            msg->id = id;
            // The low part of a long address is the full 8-bit payload.
            msg->address = payload;
            return msg;
        }
        case RailcomID::DYN: {
            DynMessage* msg = new DynMessage();
            msg->id = id;
            msg->subIndex = payload & 0x3F;
            msg->value = (payload >> 6) & 0xFF;
            return msg;
        }
        case RailcomID::XPOM_0:
        case RailcomID::XPOM_1:
        case RailcomID::XPOM_2:
        case RailcomID::XPOM_3: {
            if (bitCount == 36) { // XPOM message
                XpomMessage* msg = new XpomMessage();
                msg->id = id;
                msg->sequence = static_cast<uint8_t>(id) - static_cast<uint8_t>(RailcomID::XPOM_0);
                msg->cvValues[0] = (payload >> 24) & 0xFF;
                msg->cvValues[1] = (payload >> 16) & 0xFF;
                msg->cvValues[2] = (payload >> 8) & 0xFF;
                msg->cvValues[3] = payload & 0xFF;
                return msg;
            } else { // STAT2 message
                Stat2Message* msg = new Stat2Message();
                msg->id = RailcomID::STAT2;
                msg->status = payload;
                return msg;
            }
        }
        case RailcomID::STAT1: {
            Stat1Message* msg = new Stat1Message();
            msg->id = id;
            msg->status = payload;
            return msg;
        }
        case RailcomID::STAT4: {
            Stat4Message* msg = new Stat4Message();
            msg->id = id;
            msg->status = payload;
            return msg;
        }
        case RailcomID::ERROR: {
            ErrorMessage* msg = new ErrorMessage();
            msg->id = id;
            msg->errorCode = payload;
            return msg;
        }
        case RailcomID::TIME: {
            TimeMessage* msg = new TimeMessage();
            msg->id = id;
            msg->resolution = (payload >> 7) & 0x01;
            msg->time = payload & 0x7F;
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
