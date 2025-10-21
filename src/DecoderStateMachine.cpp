#include "DecoderStateMachine.h"

DecoderStateMachine::DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address)
    : _txManager(txManager), _type(type), _address(address), _last_addressed_time(0) {}

void DecoderStateMachine::handleDccPacket(const DCCMessage& dccMsg) {
    const uint8_t* data = dccMsg.getData();
    size_t len = dccMsg.getLength();

    // This is a simplified DCC parser. A real implementation would be more robust.
    uint16_t msg_address = 0;
    if (len >= 2) {
        msg_address = (data[0] << 8) | data[1];
    }

    if (_type == DecoderType::LOCOMOTIVE && msg_address == _address) {
        _last_addressed_time = millis();

        // Check for POM read command (simplified check)
        if (len >= 4 && (data[2] & 0b11101100) == 0b11101100) {
            uint8_t cv = data[3];
            uint8_t value = 42; // Dummy value
            _txManager.sendPomResponse(value);
        } else {
            // Otherwise, just broadcast address
            _txManager.sendAddress(_address);
        }
    }
    else if (_type == DecoderType::ACCESSORY && msg_address == _address) {
        // For an accessory, just send a status update
        _txManager.sendStatus1(0); // Dummy status
    }
}
