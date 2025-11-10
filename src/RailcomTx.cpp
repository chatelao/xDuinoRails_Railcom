/**
 * @file RailcomTx.cpp
 * @brief Implementation of the RailcomTx class for transmitting RailCom messages.
 */
#include "RailcomTx.h"
#include "RailcomEncoding.h"
#include "RailcomProtocolDefs.h"
#include "pico/time.h"
#include <cstring>

/**
 * @brief Constructs a RailcomTx object.
 * @param hardware A pointer to a RailcomTxHardware implementation.
 */
RailcomTx::RailcomTx(RailcomTxHardware* hardware)
    : _hardware(hardware), _address_alternator(0), _info1_enabled(false), _info1_payload(0) {
}

/**
 * @brief Initializes the hardware for transmission.
 */
void RailcomTx::begin() {
    _hardware->begin();
}

/**
 * @brief Deinitializes the hardware.
 */
void RailcomTx::end() {
    _hardware->end();
}

/**
 * @brief Triggers the transmission of queued messages at the start of a DCC cutout.
 * @details This function sends one message from the Channel 1 queue, waits for the
 *          appropriate Channel 2 delay, and then sends all messages from the Channel 2 queue.
 * @param elapsed_us The time in microseconds since the last cutout started.
 */
void RailcomTx::on_cutout_start(uint32_t elapsed_us) {
    if (!_ch1_queue.empty()) {
        const auto& msg = _ch1_queue.front();
        _hardware->send_bytes(msg);
        _ch1_queue.pop();
    }

    if (elapsed_us < RAILCOM_CH2_DELAY_US) {
        sleep_us(RAILCOM_CH2_DELAY_US - elapsed_us);
    }

    while (!_ch2_queue.empty()) {
        const auto& msg = _ch2_queue.front();
        _hardware->send_bytes(msg);
        _ch2_queue.pop();
    }
}

/**
 * @brief Encodes a message and adds it to the appropriate transmission queue.
 * @param channel The channel (1 or 2) to queue the message for.
 * @param id The RailcomID of the message.
 * @param payload The raw payload data.
 * @param payloadBits The number of bits in the payload.
 */
void RailcomTx::sendDatagram(uint8_t channel, RailcomID id, uint64_t payload, uint8_t payloadBits) {
    auto encodedMsg = RailcomEncoding::encodeDatagram(id, payload, payloadBits);
    if (channel == 1) {
        _ch1_queue.push(encodedMsg);
    } else {
        _ch2_queue.push(encodedMsg);
    }
}

/**
 * @brief Queues a POM response on Channel 2.
 * @param cvValue The 8-bit value of the CV.
 */
void RailcomTx::sendPomResponse(uint8_t cvValue) {
    sendDatagram(2, RailcomID::POM, cvValue, 8);
}

/**
 * @brief Queues a driving information (INFO) message on Channel 2.
 * @param speed The current speed.
 * @param motorLoad The current motor load.
 * @param statusFlags Additional status flags.
 */
void RailcomTx::sendInfo(uint16_t speed, uint8_t motorLoad, uint8_t statusFlags) {
    uint32_t payload = 0;
    payload |= (uint32_t)speed << 16;
    payload |= (uint32_t)motorLoad << 8;
    payload |= (uint32_t)statusFlags;
    sendDatagram(2, RailcomID::INFO, payload, 32);
}

/**
 * @brief Queues an extended (EXT) message on Channel 2.
 * @param type The message type.
 * @param position The position value.
 */
void RailcomTx::sendExt(uint8_t type, uint8_t position) {
    uint16_t payload = ((type & 0x07) << 8) | position;
    sendDatagram(2, RailcomID::EXT, payload, 14);
}

/**
 * @brief Queues an address broadcast message on Channel 1.
 * @details This function implements a state machine to alternate between sending
 *          ADR_HIGH, ADR_LOW, and optionally INFO1 messages. It correctly handles
 *          both short and long DCC addresses.
 * @param address The decoder's address.
 */
void RailcomTx::sendAddress(uint16_t address) {
    uint8_t max_alternator = _info1_enabled ? 3 : 2;

    switch (_address_alternator) {
        case 0: // ADR_HIGH
            if (address >= MIN_SHORT_ADDRESS && address <= MAX_SHORT_ADDRESS) {
                sendDatagram(1, RailcomID::ADR_HIGH, 0, 8);
            } else {
                sendDatagram(1, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
            }
            break;
        case 1: // ADR_LOW
            if (address >= MIN_SHORT_ADDRESS && address <= MAX_SHORT_ADDRESS) {
                sendDatagram(1, RailcomID::ADR_LOW, address & 0x7F, 8);
            } else {
                sendDatagram(1, RailcomID::ADR_LOW, address & 0xFF, 8);
            }
            break;
        case 2: // INFO1
            if (_info1_enabled) {
                sendDatagram(1, RailcomID::INFO1, _info1_payload, 8);
            }
            break;
    }
    _address_alternator = (_address_alternator + 1) % max_alternator;
}

/**
 * @brief Constructs the 8-bit payload for an INFO1 message from the struct's boolean flags.
 * @param info1 The Info1Message data.
 * @return The calculated 8-bit payload.
 */
uint8_t RailcomTx::buildInfo1Payload(const Info1Message& info1) {
    uint8_t payload = 0;
    payload |= (info1.on_track_direction_is_positive ? 1 : 0) << 0;
    payload |= (info1.travel_direction_is_positive ? 1 : 0) << 1;
    payload |= (info1.is_moving ? 1 : 0) << 2;
    payload |= (info1.is_in_consist ? 1 : 0) << 3;
    payload |= (info1.request_addressing ? 1 : 0) << 4;
    return payload;
}

/**
 * @brief Enables the INFO1 message in the Channel 1 address broadcast cycle.
 * @param info1 The Info1Message data to be sent.
 */
void RailcomTx::enableInfo1(const Info1Message& info1) {
    _info1_payload = buildInfo1Payload(info1);
    _info1_enabled = true;
}

/**
 * @brief Disables the INFO1 message in the Channel 1 address broadcast cycle.
 */
void RailcomTx::disableInfo1() {
    _info1_enabled = false;
    if (_address_alternator > 1) {
        _address_alternator = 0;
    }
}

/**
 * @brief Queues a dynamic data (DYN) message on Channel 2.
 * @param subIndex The sub-index of the data.
 * @param value The value of the data.
 */
void RailcomTx::sendDynamicData(uint8_t subIndex, uint8_t value) {
    uint16_t payload = (value << 6) | (subIndex & 0x3F);
    sendDatagram(2, RailcomID::DYN, payload, 14);
}

/**
 * @brief Queues an automatic CV broadcast (CV_AUTO) message on Channel 2.
 * @param cvAddress The full address of the CV.
 * @param cvValue The value of the CV.
 */
void RailcomTx::sendCvAuto(uint32_t cvAddress, uint8_t cvValue) {
    uint32_t payload = ((cvAddress & 0xFFFFFF) << 8) | cvValue;
    sendDatagram(2, RailcomID::CV_AUTO, payload, 32);
}

/**
 * @brief Queues an extended POM (XPOM) response on Channel 2.
 * @param sequence The sequence number (0-3) of the message part.
 * @param cvValues A pointer to an array of 4 CV values.
 */
void RailcomTx::sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]) {
    if (sequence > MAX_XPOM_SEQUENCE) return;
    RailcomID id = static_cast<RailcomID>(static_cast<uint8_t>(RailcomID::XPOM_0) + sequence);
    uint32_t payload = (uint32_t)cvValues[0] << 24 | (uint32_t)cvValues[1] << 16 | (uint32_t)cvValues[2] << 8 | cvValues[3];
    sendDatagram(2, id, payload, 32);
}

/**
 * @brief Queues the messages for a rerailing search response on Channel 2.
 * @param address The decoder's address.
 * @param secondsSincePowerOn The time since power on, capped at 255.
 */
void RailcomTx::handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn) {
    sendDatagram(2, RailcomID::ADR_HIGH, (address >> 8) & 0x3F, 6);
    sendDatagram(2, RailcomID::ADR_LOW, address & 0xFF, 8);
    sendDatagram(2, RailcomID::RERAIL, secondsSincePowerOn > MAX_RERAIL_SECONDS ? MAX_RERAIL_SECONDS : secondsSincePowerOn, 8);
}

/**
 * @brief Queues a block (BLOCK) message on Channel 2.
 * @param data The 32-bit data payload.
 */
void RailcomTx::sendBlock(uint32_t data) {
    sendDatagram(2, RailcomID::BLOCK, data, 32);
}

/**
 * @brief Queues a service request (SRQ) message on Channel 1.
 * @details This uses a special encoding function as SRQ has a unique format.
 * @param accessoryAddress The address of the accessory.
 * @param isExtended True if the address is an extended accessory address.
 */
void RailcomTx::sendServiceRequest(uint16_t accessoryAddress, bool isExtended) {
    if (accessoryAddress > MAX_ACCESSORY_ADDRESS) return;
    _ch1_queue.push(RailcomEncoding::encodeServiceRequest(accessoryAddress, isExtended));
}

/**
 * @brief Queues a status message for 1 turnout (STAT1) on Channel 2.
 * @param status The 8-bit status payload.
 */
void RailcomTx::sendStatus1(uint8_t status) {
    sendDatagram(2, RailcomID::STAT1, status, 8);
}

/**
 * @brief Queues a status message for 4 turnouts (STAT4) on Channel 2.
 * @param status The 8-bit status payload.
 */
void RailcomTx::sendStatus4(uint8_t status) {
    sendDatagram(2, RailcomID::STAT4, status, 8);
}

/**
 * @brief Queues a status message for 2 turnouts (STAT2) on Channel 2.
 * @param status The 8-bit status payload.
 */
void RailcomTx::sendStatus2(uint8_t status) {
    sendDatagram(2, RailcomID::STAT2, status, 8);
}

/**
 * @brief Queues a time (TIME) message on Channel 2.
 * @param timeValue The time value (0-127).
 * @param unit_is_second If true, the unit is seconds; otherwise, it's 0.1 seconds.
 */
void RailcomTx::sendTime(uint8_t timeValue, bool unit_is_second) {
    if (timeValue > 127) timeValue = 127;
    uint8_t payload = (unit_is_second ? 0x80 : 0x00) | timeValue;
    sendDatagram(2, RailcomID::TIME, payload, 8);
}

/**
 * @brief Queues an error (ERROR) message on Channel 2.
 * @param errorCode The 8-bit error code.
 */
void RailcomTx::sendError(uint8_t errorCode) {
    sendDatagram(2, RailcomID::ERROR, errorCode, 8);
}

/**
 * @brief Queues a decoder unique ID (DECODER_UNIQUE) message on Channel 2.
 * @param manufacturerId The 16-bit manufacturer ID.
 * @param productId The 32-bit product ID.
 */
void RailcomTx::sendDecoderUnique(uint16_t manufacturerId, uint32_t productId) {
    uint64_t payload = ((uint64_t)(manufacturerId & 0x0FFF) << 32) | (productId & 0xFFFFFFFF);
    sendDatagram(2, RailcomID::DECODER_UNIQUE, payload, 44);
}

/**
 * @brief Queues a decoder state (DECODER_STATE) message on Channel 2.
 * @param changeFlags Flags indicating state changes.
 * @param changeCount A counter that increments on each change.
 * @param protocolCaps Flags indicating protocol capabilities.
 */
void RailcomTx::sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps) {
    uint64_t payload = 0;
    payload |= (uint64_t)(changeFlags & 0xFF) << 36;
    payload |= (uint64_t)(changeCount & 0x0FFF) << 24;
    payload |= (uint64_t)(protocolCaps & 0xFFFF) << 8;
    // 8 bits reserved
    sendDatagram(2, RailcomID::DECODER_STATE, payload, 44);
}

/**
 * @brief Queues a Data Space message.
 * @details This is a special message type that doesn't use the standard datagram
 *          format. It performs a 4-of-8 encoding on the raw data and its CRC, then
 *          splits the result across Channel 1 and Channel 2 as per RCN-218.
 * @param data Pointer to the data payload.
 * @param len The length of the data.
 * @param dataSpaceNum The data space number, used to seed the CRC.
 */
void RailcomTx::sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum) {
    uint8_t header = (len & DATASPACE_LEN_MASK);
    uint8_t buffer[33];
    buffer[0] = header;
    memcpy(buffer + 1, data, len);
    uint8_t crc = RailcomEncoding::crc8(buffer, len + 1, dataSpaceNum);
    std::vector<uint8_t> encodedBytes;
    encodedBytes.reserve(len + 2);

    // Encode header, data, and CRC using 4-of-8 encoding
    encodedBytes.push_back(RailcomEncoding::encode4of8(header));
    for (size_t i = 0; i < len; ++i) {
        encodedBytes.push_back(RailcomEncoding::encode4of8(data[i]));
    }
    encodedBytes.push_back(RailcomEncoding::encode4of8(crc));

    // Per RCN-218, Data Space messages are split across channels.
    // The first two bytes go to Channel 1, the rest to Channel 2.
    if (encodedBytes.size() >= 2) {
        _ch1_queue.push(std::vector<uint8_t>(encodedBytes.begin(), encodedBytes.begin() + 2));
        if (encodedBytes.size() > 2) {
            _ch2_queue.push(std::vector<uint8_t>(encodedBytes.begin() + 2, encodedBytes.end()));
        }
    } else if (!encodedBytes.empty()) {
        // Handle cases with less than 2 bytes, though unlikely for this message type.
        _ch1_queue.push(encodedBytes);
    }
}

/**
 * @brief Queues the ACK signal bytes on both channels.
 */
void RailcomTx::sendAck() {
    std::vector<uint8_t> ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2 };
    _ch1_queue.push(ackBytes);
    ackBytes = { RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2 };
    _ch2_queue.push(ackBytes);
}

/**
 * @brief Queues the NACK signal bytes on both channels.
 */
void RailcomTx::sendNack() {
    std::vector<uint8_t> nackBytes = { RAILCOM_NACK, RAILCOM_NACK };
    _ch1_queue.push(nackBytes);
    nackBytes = { RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK };
    _ch2_queue.push(nackBytes);
}
