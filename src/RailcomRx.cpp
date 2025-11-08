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

void RailcomRx::setContext(DecoderContext context) {
    _context = context;
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
        case RailcomID::EXT: // Also covers STAT4
            if (_lastRawBytes.size() * 6 - 4 == 14) { // EXT message has 14 payload bits -> 18 total bits -> 3 bytes
                ExtMessage* msg = static_cast<ExtMessage*>(_lastMessage);
                stream.print("  ID: EXT (3)\n");
                stream.printf("  Type: %u\n", msg->type);
                stream.printf("  Position: %u\n", msg->position);
            } else if (_lastMessage->id == RailcomID::INFO1) {
                Info1Message* msg = static_cast<Info1Message*>(_lastMessage);
                stream.print("  ID: INFO1 (3)\n");
                stream.printf("  On-track direction positive: %d\n", msg->on_track_direction_is_positive);
                stream.printf("  Travel direction positive: %d\n", msg->travel_direction_is_positive);
                stream.printf("  Is moving: %d\n", msg->is_moving);
                stream.printf("  Is in consist: %d\n", msg->is_in_consist);
                stream.printf("  Request addressing: %d\n", msg->request_addressing);
            } else { // STAT4 message has 8 payload bits -> 12 total bits -> 2 bytes
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
            }
            break;
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
        case RailcomID::DECODER_STATE: {
            DecoderStateMessage* msg = static_cast<DecoderStateMessage*>(_lastMessage);
            stream.print("  ID: DECODER_STATE (13)\n");
            stream.printf("  Change Flags: %u\n", msg->changeFlags);
            stream.printf("  Change Count: %u\n", msg->changeCount);
            stream.printf("  Protocol Caps: %u\n", msg->protocolCaps);
            break;
        }
        case RailcomID::RERAIL:
            stream.print("  ID: RERAIL/SRQ (14)\n");
            // Cannot distinguish between RERAIL and SRQ here, so printing raw payload.
            break;
        case RailcomID::DECODER_UNIQUE: {
            DecoderUniqueMessage* msg = static_cast<DecoderUniqueMessage*>(_lastMessage);
            stream.print("  ID: DECODER_UNIQUE (15)\n");
            stream.printf("  Manufacturer ID: %u\n", msg->manufacturerId);
            stream.printf("  Product ID: %lu\n", msg->productId);
            break;
        }
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
        case RailcomID::STAT1: {
            Stat1Message* msg = new Stat1Message();
            msg->id = id;
            msg->status = payload;
            return msg;
        }
        case RailcomID::EXT: { // Also STAT4 and INFO1
            if (bitCount == 18) { // EXT Message (currently no context needed)
                ExtMessage* msg = new ExtMessage();
                msg->id = RailcomID::EXT;
                uint8_t type = (payload >> 8) & 0x0F;
                // The type must be in the range 0-7.
                if (type > 7) return nullptr;
                msg->type = type;
                msg->position = payload & 0xFF;
                return msg;
            } else { // INFO1 or STAT4 Message (8 payload bits)
                if (_context == DecoderContext::MOBILE) {
                    Info1Message* msg = new Info1Message();
                    msg->id = RailcomID::INFO1;
                    msg->on_track_direction_is_positive = (payload >> 0) & 1;
                    msg->travel_direction_is_positive = (payload >> 1) & 1;
                    msg->is_moving = (payload >> 2) & 1;
                    msg->is_in_consist = (payload >> 3) & 1;
                    msg->request_addressing = (payload >> 4) & 1;
                    return msg;
                } else { // Default to STATIONARY or if UNKNOWN
                    Stat4Message* msg = new Stat4Message();
                    msg->id = RailcomID::STAT4;
                    msg->status = payload;
                    return msg;
                }
            }
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
            msg->protocolCaps = (payload >> 8) & 0xFFFF;
            msg->changeCount = (payload >> 24) & 0x0FFF;
            msg->changeFlags = (payload >> 36) & 0xFF;
            return msg;
        }
        case RailcomID::RERAIL: // and SRQ
            if (bitCount - 4 == 12) { // SRQ
                SrqMessage* msg = new SrqMessage();
                msg->id = RailcomID::SRQ;
                msg->isExtended = (payload >> 11) & 0x01;
                msg->accessoryAddress = payload & 0x7FF;
                return msg;
            } else { // RERAIL
                RerailMessage* msg = new RerailMessage();
                msg->id = RailcomID::RERAIL;
                msg->counter = payload;
                return msg;
            }
        case RailcomID::DECODER_UNIQUE: {
            DecoderUniqueMessage* msg = new DecoderUniqueMessage();
            msg->id = id;
            msg->productId = payload & 0xFFFFFFFF;
            msg->manufacturerId = (payload >> 32) & 0x0FFF;
            return msg;
        }
        default:
            return nullptr;
    }
}
