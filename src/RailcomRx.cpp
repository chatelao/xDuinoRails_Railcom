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

RailcomMessage* RailcomRx::read() {
    // Clear previous message
    if (_lastMessage != nullptr) {
        delete _lastMessage;
        _lastMessage = nullptr;
    }
    _lastRawBytes.clear();

    if (read_raw_bytes(_lastRawBytes, 50)) {
        _lastMessage = parseMessage(_lastRawBytes);
        return _lastMessage;
    }
    return nullptr;
}

void RailcomRx::print(Print& stream) {
    if (_lastMessage == nullptr) {
        stream.println("No Railcom message received.");
        return;
    }

    stream.print("Raw bytes: ");
    for (size_t i = 0; i < _lastRawBytes.size(); ++i) {
        char buf[4];
        sprintf(buf, "%02X ", _lastRawBytes[i]);
        stream.print(buf);
    }
    stream.println();

    stream.println("Decoded data:");
    switch (_lastMessage->id) {
        case RailcomID::POM:
            stream.print("  ID: POM (0)\n");
            stream.printf("  CV Value: %u\n", static_cast<PomMessage*>(_lastMessage)->cvValue);
            break;
        case RailcomID::ADR_HIGH: {
            uint16_t adrPart = static_cast<AdrMessage*>(_lastMessage)->address;
            _lastAdrHigh = adrPart;
            stream.print("  ID: ADR_HIGH (1)\n");
            stream.printf("  Address part: %u\n", adrPart);
            break;
        }
        case RailcomID::ADR_LOW: {
            uint16_t adrPart = static_cast<AdrMessage*>(_lastMessage)->address;
            stream.print("  ID: ADR_LOW (2)\n");
            stream.printf("  Address part: %u\n", adrPart);
            if (_lastAdrHigh != 0) {
                uint16_t fullAddress = (_lastAdrHigh << 8) | adrPart;
                stream.printf("  Effective Address: %u\n", fullAddress);
                _lastAdrHigh = 0; // Reset after use
            }
            break;
        }
        case RailcomID::STAT4: {
             stream.print("  ID: STAT4 (3)\n");
             uint8_t status = static_cast<Stat4Message*>(_lastMessage)->status;
             stream.println("  Turnout Status:");
             for (int i = 3; i >= 0; i--) {
                 const int turnoutNum = i + 1;
                 const bool greenBit = (status >> (i * 2 + 1)) & 1;
                 const bool redBit = (status >> (i * 2)) & 1;
                 stream.printf("    Pair %d: ", turnoutNum);
                 if (greenBit) {
                     stream.println("Green (straight/right/go)");
                 } else if (redBit) {
                     stream.println("Red (turn/left/stop)");
                 } else {
                     stream.println("Off");
                 }
             }
             break;
        }
        case RailcomID::SRQ: {
            SrqMessage* msg = static_cast<SrqMessage*>(_lastMessage);
            stream.print("  ID: SRQ (4)\n");
            stream.printf("  Accessory Address: %u\n", msg->accessoryAddress);
            stream.printf("  Is Extended: %s\n", msg->isExtended ? "Yes" : "No");
            break;
        }
        case RailcomID::DYN: {
             DynMessage* msg = static_cast<DynMessage*>(_lastMessage);
             stream.print("  ID: DYN (7)\n");
             stream.printf("  SubIndex: %u\n", msg->subIndex);
             stream.printf("  Value: %u\n", msg->value);
             break;
        }
        case RailcomID::TIME: {
            TimeMessage* msg = static_cast<TimeMessage*>(_lastMessage);
            stream.print("  ID: TIME (5)\n");
            if (msg->unit_is_second) {
                stream.printf("  Restlaufzeit: %u Sekunden\n", msg->timeValue);
            } else {
                stream.printf("  Restlaufzeit: %.1f Sekunden\n", msg->timeValue / 10.0);
            }
            break;
        }
        case RailcomID::DECODER_STATE:
            stream.print("  ID: DECODER_STATE (13)\n");
            stream.printf("  Application-specific state: %lu\n", static_cast<DecoderStateMessage*>(_lastMessage)->state);
            break;
        case RailcomID::RERAIL:
            stream.print("  ID: RERAIL (14)\n");
            stream.printf("  Rerail counter: %u\n", static_cast<RerailMessage*>(_lastMessage)->counter);
            break;
        case RailcomID::DECODER_UNIQUE:
            stream.print("  ID: DECODER_UNIQUE (15)\n");
            stream.printf("  Unique ID part: %lu\n", static_cast<DecoderUniqueMessage*>(_lastMessage)->uniqueId);
            break;
        default:
            stream.printf("  ID: %d (Unknown)\n", static_cast<int>(_lastMessage->id));
            break;
    }
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
        case RailcomID::STAT1: { // Also handles SRQ and INFO
            if (bitCount == 12) { // 4 ID bits + 8 payload bits
                Stat1Message* msg = new Stat1Message();
                msg->id = id;
                msg->status = payload;
                return msg;
            } else if (bitCount == 18) { // 4 ID bits + 14 payload bits (SRQ uses 12)
                SrqMessage* msg = new SrqMessage();
                msg->id = RailcomID::SRQ;
                msg->accessoryAddress = payload & 0x7FF;
                msg->isExtended = (payload >> 11) & 0x01;
                return msg;
            }
            return nullptr; // Unknown length for this ID
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
            msg->unit_is_second = (payload >> 7) & 0x01;
            msg->timeValue = payload & 0x7F;
            return msg;
        }
        case RailcomID::CV_AUTO: {
            CvAutoMessage* msg = new CvAutoMessage();
            msg->id = id;
            msg->cvAddress = (payload >> 8) & 0xFFFFFF;
            msg->cvValue = payload & 0xFF;
            return msg;
        }
        case RailcomID::DECODER_STATE: {
            DecoderStateMessage* msg = new DecoderStateMessage();
            msg->id = id;
            msg->state = payload;
            return msg;
        }
        case RailcomID::RERAIL: {
            RerailMessage* msg = new RerailMessage();
            msg->id = id;
            msg->counter = payload;
            return msg;
        }
        case RailcomID::DECODER_UNIQUE: {
            DecoderUniqueMessage* msg = new DecoderUniqueMessage();
            msg->id = id;
            msg->uniqueId = payload;
            return msg;
        }
        default:
            return nullptr;
    }
}
