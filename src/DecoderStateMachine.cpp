#include "DecoderStateMachine.h"

DecoderStateMachine::DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address)
    : _txManager(txManager), _type(type), _address(address), _last_addressed_time(0) {}

void DecoderStateMachine::handleDccPacket(const DCCMessage& dccMsg) {
    const uint8_t* data = dccMsg.getData();
    size_t len = dccMsg.getLength();

    // Simplified DCC address parsing
    uint16_t msg_address = 0;
    if (len >= 2) {
        // Basic Accessory Decoders have a different address format
        if (_type == DecoderType::ACCESSORY) {
            // Address is encoded in the first and second bytes
            msg_address = 1 + (((~data[0]) & 0x3F) << 2) | ((data[1] >> 1) & 0x03);
        } else {
            msg_address = (data[0] << 8) | data[1];
        }
    }

    if (msg_address != _address) {
        return; // Not for us
    }

    // --- Packet is for this decoder, decide on a response ---
    _last_addressed_time = millis();

    if (_type == DecoderType::LOCOMOTIVE) {
        if (len >= 4 && (data[2] & 0b11101100) == 0b11101100) { // POM Read
            uint8_t value = 42; // Dummy value for the requested CV
            _txManager.sendPomResponse(value);
        } else {
            _txManager.sendAddress(_address);
        }
    }
    else if (_type == DecoderType::ACCESSORY) {
        if (len >= 2) {
            bool activate = (data[1] >> 3) & 1;
            uint8_t output = data[1] & 0x03;

            // For a STAT1 message, the payload is an 8-bit status.
            // We can simulate the state of the outputs.
            // Let's say bit 0 is output 0, bit 1 is output 1, etc.
            static uint8_t accessory_state = 0;
            if (activate) {
                accessory_state |= (1 << output);
            } else {
                accessory_state &= ~(1 << output);
            }

            _txManager.sendStatus1(accessory_state);
        }
    }
}
