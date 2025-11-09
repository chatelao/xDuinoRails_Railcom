#include "DecoderStateMachine.h"

#include <Arduino.h>

DecoderStateMachine::DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint8_t cv28, uint8_t cv29, uint16_t manufacturerId, uint32_t productId)
    : _txManager(txManager), _type(type), _address(address), _cv28(cv28), _cv29(cv29),
      _manufacturerId(manufacturerId), _productId(productId), _logonState(LogonState::IDLE), _accessory_state(0), _channel1_broadcast_enabled(true) {
    setupCallbacks();
}

void DecoderStateMachine::handleDccPacket(const DCCMessage& msg) {
    // According to NMRA S-9.2.2, CV29, Bit 3 enables/disables RailCom.
    // If RailCom is not enabled, do not process any packets.
    if ((_cv29 & 0b00001000) == 0) {
        return;
    }

    bool response_sent = false;
    _dccParser.parse(msg, &response_sent);

    // Also send address as a default response for locomotives
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();
    bool is_addressed_to_me = (len >= 2 && ((data[0] << 8) | data[1]) == _address);

    if (is_addressed_to_me) {
        // Once we are addressed directly, we should stop broadcasting on Ch1
        // to reduce channel congestion.
        _channel1_broadcast_enabled = false;
    }

    if (!response_sent && _type == DecoderType::LOCOMOTIVE && _channel1_broadcast_enabled) {
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

    // Handles an extended function command by sending an appropriate response.
    // See RCN-217 Section 4.3.1 and 5.2.3
    _dccParser.onExtendedFunction = [this](uint16_t address, uint8_t command) {
        // XF2 (Rerailing Search) is a broadcast command sent to address 0.
        // See RCN-217 Section 5.2.3
        if (command == 0x02 && address == 0) {
            // Respond with address and time since power-on.
            // In a real application, `millis()` would be replaced with a timer
            // that is reset on power-on.
            _txManager.handleRerailingSearch(_address, millis() / 1000);
            return; // Explicitly return to avoid falling through to addressed commands
        }

        if (address == _address) {
            // XF1 is the command for "request for location information"
            // See RCN-217 Section 5.3.1
            if (command == 0x01) {
                // Send a dummy EXT message with type 0 and position 0.
                // In a real application, these values would come from sensors.
                _txManager.sendExt(0, 0);
            }
        }
    };
}
