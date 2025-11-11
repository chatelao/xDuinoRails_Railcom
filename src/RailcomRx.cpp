/**
 * @file RailcomRx.cpp
 * @brief Implementation of the RailcomRx class for receiving and parsing RailCom messages.
 */
#include "RailcomRx.h"
#include "RailcomEncoding.h"
#include <cstring>
#include "pico/stdlib.h"

/**
 * @brief Constructs a RailcomRx object.
 * @param hardware A pointer to a RailcomHardware implementation.
 */
RailcomRx::RailcomRx(RailcomRxHardware* hardware)
    : _hardware(hardware) {}

/**
 * @brief Initializes the hardware for reception.
 */
void RailcomRx::begin() {
    _hardware->begin();
}

/**
 * @brief A periodic task function, currently not used.
 */
void RailcomRx::task() {
    // task() is not used in the restored logic,
    // so this is intentionally left empty.
}

/**
 * @brief Reads a sequence of raw bytes from the hardware.
 * @details This is a private helper function that reads all available bytes from
 *          the hardware buffer until it's empty, or until the timeout is reached.
 * @param[out] buffer The vector to store the read bytes.
 * @param timeout_ms The maximum time to wait for data.
 * @return True if at least one byte was read, false otherwise.
 */
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

/**
 * @brief Sets the decoder context for parsing ambiguous messages.
 * @param context The decoder context (MOBILE or STATIONARY).
 */
void RailcomRx::setContext(DecoderContext context) {
    _context = context;
}

/**
 * @brief Configures the receiver to expect a special Data Space message next.
 * @param dataSpaceNum The expected data space number, used for CRC validation.
 */
void RailcomRx::expectDataSpaceResponse(uint8_t dataSpaceNum) {
    _is_data_space_expected = true;
    _expected_data_space_num = dataSpaceNum;
}

/**
 * @brief Reads, parses, and returns the next available RailCom message.
 * @details This is the main public method for receiving messages. It handles two main
 *          cases:
 *          1. If `expectDataSpaceResponse` was called, it uses a special parsing
 *             logic for RCN-218 Data Space messages.
 *          2. Otherwise, it uses the general `parseMessage` function for all
 *             standard RCN-217 and RCN-218 messages.
 *          The returned pointer is managed internally and will be deleted on the
 *          next call to `read()`.
 * @return A pointer to a parsed RailcomMessage, or nullptr if no valid message is received.
 */
RailcomMessage* RailcomRx::read() {
    // Clear previous message
    if (_lastMessage != nullptr) {
        delete _lastMessage;
        _lastMessage = nullptr;
    }
    _lastRawBytes.clear();

    if (read_raw_bytes(_lastRawBytes, 50)) {
        if (_is_data_space_expected) {
            _is_data_space_expected = false; // Consume the expectation
            // Data Space messages have no ID, they are raw byte streams.
            // The first byte is the length.
            if (_lastRawBytes.empty()) return nullptr;

            std::vector<uint8_t> decoded_payload;
            for (uint8_t byte : _lastRawBytes) {
                decoded_payload.push_back(RailcomEncoding::decode4of8(byte));
            }

            if (decoded_payload.empty()) return nullptr;

            uint8_t len = decoded_payload[0];
            if (len > MAX_DATA_SPACE_PAYLOAD || decoded_payload.size() != (size_t)len + 2) {
                // Invalid length or size mismatch
                return nullptr;
            }

            DataSpaceMessage* msg = new DataSpaceMessage();
            msg->id = (RailcomID) -1; // Special ID for data space
            msg->len = len;
            memcpy(msg->data, decoded_payload.data() + 1, len);
            msg->crc = decoded_payload.back();
            msg->dataSpaceNum = _expected_data_space_num;

            // Verify CRC
            uint8_t crc_buffer[MAX_DATA_SPACE_PAYLOAD + 1];
            crc_buffer[0] = len;
            memcpy(crc_buffer + 1, msg->data, len);
            uint8_t calculated_crc = RailcomEncoding::crc8(crc_buffer, len + 1, msg->dataSpaceNum);
            msg->crc_ok = (calculated_crc == msg->crc);

            _lastMessage = msg;
            return msg;
        } else {
            _lastMessage = parseMessage(_lastRawBytes);
            return _lastMessage;
        }
    }
    return nullptr;
}

/**
 * @brief Prints a formatted, human-readable version of the last message to a Print stream.
 * @param stream The Arduino Print stream (e.g., `Serial`) to write to.
 */
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

    // Handle Data Space as a special case because it has a special ID
    if (_lastMessage->id == (RailcomID)-1) {
        DataSpaceMessage* dsMsg = static_cast<DataSpaceMessage*>(_lastMessage);
        stream.print("  ID: DATA_SPACE\n");
        stream.printf("  Data Space Num: %u\n", dsMsg->dataSpaceNum);
        stream.printf("  Length: %u\n", dsMsg->len);
        stream.print("  Data: ");
        for(int i=0; i<dsMsg->len; i++) {
            char buf[4];
            sprintf(buf, "%02X ", dsMsg->data[i]);
            stream.print(buf);
        }
        stream.println();
        stream.printf("  CRC: 0x%02X (Received)\n", dsMsg->crc);
        stream.printf("  CRC OK: %s\n", dsMsg->crc_ok ? "Yes" : "No");
        return;
    }

    stream.println("Decoded data:");
    switch (_lastMessage->id) {
        case RailcomID::POM:
            stream.print("  ID: POM (0)\n");
            stream.printf("  CV Value: %u\n", static_cast<PomMessage*>(_lastMessage)->cvValue);
            break;
        case RailcomID::INFO: {
            InfoMessage* msg = static_cast<InfoMessage*>(_lastMessage);
            stream.print("  ID: INFO (4)\n");
            stream.printf("  Speed: %u\n", msg->speed);
            stream.printf("  Motor Load: %u\n", msg->motorLoad);
            stream.printf("  Status Flags: 0x%02X\n", msg->statusFlags);
            break;
        }
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
        case RailcomID::EXT: // Also covers STAT4 and INFO1
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
        case RailcomID::DECODER_STATE: { // Also BLOCK
            if (_lastRawBytes.size() == 8) { // 44-bit payload -> 48 total bits -> 8 bytes
                DecoderStateMessage* msg = static_cast<DecoderStateMessage*>(_lastMessage);
                stream.print("  ID: DECODER_STATE (13)\n");
                stream.printf("  Change Flags: %u\n", msg->changeFlags);
                stream.printf("  Change Count: %u\n", msg->changeCount);
                stream.printf("  Protocol Caps: %u\n", msg->protocolCaps);
            } else if (_lastRawBytes.size() == 6) { // 32-bit payload -> 36 total bits -> 6 bytes
                BlockMessage* msg = static_cast<BlockMessage*>(_lastMessage);
                stream.print("  ID: BLOCK (13)\n");
                stream.printf("  Data: 0x%08lX\n", msg->data);
            }
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

/**
 * @brief Parses a vector of raw bytes into a specific RailcomMessage struct.
 * @details This is the core parsing logic. It performs the following steps:
 *          1. Decodes the 4-of-8 encoded bytes into a single 64-bit integer.
 *          2. Extracts the 4-bit message ID from the start of the data.
 *          3. Extracts the payload.
 *          4. Uses a switch statement on the ID to create the appropriate message
 *             struct and populate it with the payload data.
 *          It handles messages with variable lengths and ambiguous IDs by checking
 *          the total bit count and the current decoder context.
 * @param buffer A constant reference to the vector of raw bytes to parse.
 * @return A pointer to a newly allocated message struct, or nullptr if parsing fails.
 */
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
        case RailcomID::POM: { // RCN-217, 5.2.1
            PomMessage* msg = new PomMessage();
            msg->id = id;
            msg->cvValue = payload;
            return msg;
        }
        case RailcomID::ADR_HIGH: { // RCN-217, 5.2.2
            AdrMessage* msg = new AdrMessage();
            msg->id = id;
            // Per RCN-217 for long addresses, the address is in the lower 6 bits.
            msg->address = payload & 0x3F;
            return msg;
        }
        case RailcomID::ADR_LOW: { // RCN-217, 5.2.3
            AdrMessage* msg = new AdrMessage();
            msg->id = id;
            // The low part of a long address is the full 8-bit payload.
            msg->address = payload;
            return msg;
        }
        case RailcomID::DYN: { // RCN-217, 5.2.8
            DynMessage* msg = new DynMessage();
            msg->id = id;
            msg->subIndex = payload & 0x3F;
            msg->value = (payload >> 6) & 0xFF;
            return msg;
        }
        case RailcomID::XPOM_0: // RCN-217, 5.2.9
        case RailcomID::XPOM_1:
        case RailcomID::XPOM_2:
        case RailcomID::XPOM_3: {
            if (bitCount == 36) { // XPOM message has 32 payload bits
                XpomMessage* msg = new XpomMessage();
                msg->id = id;
                msg->sequence = static_cast<uint8_t>(id) - static_cast<uint8_t>(RailcomID::XPOM_0);
                msg->cvValues[0] = (payload >> 24) & 0xFF;
                msg->cvValues[1] = (payload >> 16) & 0xFF;
                msg->cvValues[2] = (payload >> 8) & 0xFF;
                msg->cvValues[3] = payload & 0xFF;
                return msg;
            } else { // STAT2 message (RCN-217, 5.2.9) has 8 payload bits
                Stat2Message* msg = new Stat2Message();
                msg->id = RailcomID::STAT2;
                msg->status = payload;
                return msg;
            }
        }
        case RailcomID::INFO: { // RCN-217, 5.2.5
            if (bitCount == 36 && _context == DecoderContext::MOBILE) { // INFO message has 32 payload bits
                InfoMessage* msg = new InfoMessage();
                msg->id = RailcomID::INFO;
                msg->speed = (payload >> 16) & 0xFFFF;
                msg->motorLoad = (payload >> 8) & 0xFF;
                msg->statusFlags = payload & 0xFF;
                return msg;
            } else { // STAT1 message has 8 payload bits
                Stat1Message* msg = new Stat1Message();
                msg->id = RailcomID::STAT1;
                msg->status = payload;
                return msg;
            }
        }
        case RailcomID::EXT: { // RCN-217, 5.2.4
            if (bitCount == 18) { // EXT Message has 14 payload bits
                ExtMessage* msg = new ExtMessage();
                msg->id = RailcomID::EXT;
                uint8_t type = (payload >> 8) & 0x0F;
                // The type must be in the range 0-7.
                if (type > 7) return nullptr;
                msg->type = type;
                msg->position = payload & 0xFF;
                return msg;
            } else { // INFO1 or STAT4 Message (8 payload bits)
                if (_context == DecoderContext::MOBILE) { // INFO1
                    Info1Message* msg = new Info1Message();
                    msg->id = RailcomID::INFO1;
                    msg->on_track_direction_is_positive = (payload >> 0) & 1;
                    msg->travel_direction_is_positive = (payload >> 1) & 1;
                    msg->is_moving = (payload >> 2) & 1;
                    msg->is_in_consist = (payload >> 3) & 1;
                    msg->request_addressing = (payload >> 4) & 1;
                    return msg;
                } else { // STAT4 (Default for STATIONARY or UNKNOWN)
                    Stat4Message* msg = new Stat4Message();
                    msg->id = RailcomID::STAT4;
                    msg->status = payload;
                    return msg;
                }
            }
        }
        case RailcomID::ERROR: { // RCN-217, 5.2.7
            ErrorMessage* msg = new ErrorMessage();
            msg->id = id;
            msg->errorCode = payload;
            return msg;
        }
        case RailcomID::TIME: { // RCN-217, 5.2.6
            TimeMessage* msg = new TimeMessage();
            msg->id = id;
            msg->unit_is_second = (payload >> 7) & 0x01;
            msg->timeValue = payload & 0x7F;
            return msg;
        }
        case RailcomID::CV_AUTO: { // RCN-217, 5.2.11
            CvAutoMessage* msg = new CvAutoMessage();
            msg->id = id;
            msg->cvAddress = (payload >> 8) & 0xFFFFFF;
            msg->cvValue = payload & 0xFF;
            return msg;
        }
        case RailcomID::DECODER_STATE: { // Also BLOCK
            if (bitCount - 4 == 44) { // DECODER_STATE (RCN-218, 4.2)
                DecoderStateMessage* msg = new DecoderStateMessage();
                msg->id = id;
                msg->protocolCaps = (payload >> 8) & 0xFFFF;
                msg->changeCount = (payload >> 24) & 0x0FFF;
                msg->changeFlags = (payload >> 36) & 0xFF;
                return msg;
            } else if (bitCount - 4 == 32) { // BLOCK (RCN-218, 4.1)
                BlockMessage* msg = new BlockMessage();
                msg->id = RailcomID::BLOCK;
                msg->data = payload;
                return msg;
            }
            return nullptr;
        }
        case RailcomID::RERAIL: { // RCN-217, 5.2.12
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
        }
        case RailcomID::DECODER_UNIQUE: { // RCN-218, 4.3
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
