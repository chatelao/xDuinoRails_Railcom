#include "DecoderStateMachine.h"

DecoderStateMachine::DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint16_t manufacturerId, uint32_t productId)
    : _txManager(txManager), _type(type), _address(address),
      _manufacturerId(manufacturerId), _productId(productId), _logonState(LogonState::IDLE), _accessory_state(0) {
    setupCallbacks();
}

void DecoderStateMachine::handleDccPacket(const DCCMessage& msg) {
    bool response_sent = false;
    _dccParser.parse(msg, &response_sent);

    // Also send address as a default response for locomotives
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();
    if (!response_sent && _type == DecoderType::LOCOMOTIVE && len >= 2 && ((data[0] << 8) | data[1]) == _address) {
        _txManager.sendAddress(_address);
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

    // RCN-217 Callbacks

    // Handles a POM Read CV command by sending a POM response with the CV value.
    // See RCN-217 Section 5.1.1
    _dccParser.onPomReadCv = [this](uint16_t cv, uint16_t address) {
        if (address == _address) {
            uint8_t value = 42; // Dummy value for the requested CV
            _txManager.sendPomResponse(value);
        }
    };

    // Handles a POM Write CV command by sending a POM response with the new CV value.
    // See RCN-217 Section 5.1.2
    _dccParser.onPomWriteCv = [this](uint16_t cv, uint8_t value, uint16_t address) {
        if (address == _address) {
            // Here you would typically write the CV value to memory
            _txManager.sendPomResponse(value);
        }
    };

    // Handles a POM Write Bit command by sending a POM response with the new CV value.
    // See RCN-217 Section 5.1.3
    _dccParser.onPomWriteBit = [this](uint16_t cv, uint8_t bit, uint8_t value, uint16_t address) {
        if (address == _address) {
            // Here you would typically write the CV bit to memory
            uint8_t currentValue = 42; // Dummy value
            if (value) {
                currentValue |= (1 << bit);
            } else {
                currentValue &= ~(1 << bit);
            }
            _txManager.sendPomResponse(currentValue);
        }
    };

    // Handles an accessory command by updating the accessory state and sending a status response.
    // See RCN-217 Section 6.3 and 6.4
    _dccParser.onAccessory = [this](uint16_t address, bool activate, uint8_t output) {
        if (address == _address) {
            if (activate) {
                _accessory_state |= (1 << output);
            } else {
                _accessory_state &= ~(1 << output);
            }

            if (_type == DecoderType::ACCESSORY_EXTENDED) {
                _txManager.sendStatus1(_accessory_state);
            } else {
                _txManager.sendStatus4(_accessory_state);
            }
        }
    };

    // Handles a function command by sending an ACK.
    // See RCN-212 Section 2.3.5
    _dccParser.onFunction = [this](uint16_t address, uint8_t function, bool state) {
         if (address == _address) {
            // Here you would typically handle the function command
            _txManager.sendAck();
        }
    };
}
