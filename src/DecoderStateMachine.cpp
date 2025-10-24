#include "DecoderStateMachine.h"

DecoderStateMachine::DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint16_t manufacturerId, uint32_t productId)
    : _txManager(txManager), _type(type), _address(address),
      _manufacturerId(manufacturerId), _productId(productId), _logonState(LogonState::IDLE) {
    setupCallbacks();
}

void DecoderStateMachine::handleDccPacket(const DCCMessage& msg) {
    _dccParser.parse(msg);

    // Simplified DCC address parsing for RCN-217
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();
    uint16_t msg_address = 0;
    if (len >= 2) {
        if (_type == DecoderType::ACCESSORY_STANDARD || _type == DecoderType::ACCESSORY_EXTENDED) {
            msg_address = 1 + (((~data[0]) & 0x3F) << 2) | ((data[1] >> 1) & 0x03);
        } else {
            msg_address = (data[0] << 8) | data[1];
        }
    }

    if (msg_address != _address) {
        return; // Not for us
    }

    // --- RCN-217 Logic ---
    if (_type == DecoderType::LOCOMOTIVE) {
        if (len >= 4 && (data[2] & 0b11101100) == 0b11101100) { // POM Read
            uint8_t value = 42; // Dummy value for the requested CV
            _txManager.sendPomResponse(value);
        } else {
            _txManager.sendAddress(_address);
        }
    }
    else if (_type == DecoderType::ACCESSORY_STANDARD || _type == DecoderType::ACCESSORY_EXTENDED) {
        if (len >= 2) {
            bool activate = (data[1] >> 3) & 1;
            uint8_t output = data[1] & 0x03;

            static uint8_t accessory_state = 0;
            if (activate) {
                accessory_state |= (1 << output);
            } else {
                accessory_state &= ~(1 << output);
            }

            if (_type == DecoderType::ACCESSORY_EXTENDED) {
                _txManager.sendStatus1(accessory_state);
            } else {
                _txManager.sendStatus4(accessory_state);
            }
        }
    }
}

void DecoderStateMachine::setupCallbacks() {
    _dccParser.onLogonEnable = [this](uint8_t group, uint16_t zid, uint8_t sessionId) {
        if (_logonState == LogonState::IDLE) {
            _logonState = LogonState::WAITING_FOR_LOGON;
        }
        if (_logonState == LogonState::WAITING_FOR_LOGON) {
            _txManager.sendDecoderUnique(_manufacturerId, _productId);
            _logonState = LogonState::IN_SINGULATION;
        }
    };

    _dccParser.onSelect = [this](uint16_t manufacturerId, uint32_t productId, uint8_t subCmd, const uint8_t* data, size_t len) {
        if (manufacturerId == _manufacturerId && productId == _productId) {
            if (_logonState == LogonState::IN_SINGULATION) {
                _logonState = LogonState::ANNOUNCED;
                // Handle sub-commands like ReadShortInfo, ReadBlock, etc.
                _txManager.sendAck();
            }
        }
    };

    _dccParser.onLogonAssign = [this](uint16_t manufacturerId, uint32_t productId, uint16_t address) {
        if (manufacturerId == _manufacturerId && productId == _productId) {
            if (_logonState == LogonState::ANNOUNCED) {
                _address = address;
                _logonState = LogonState::REGISTERED;
                _txManager.sendDecoderState(0, 0, 0); // Dummy values
            }
        }
    };
}
