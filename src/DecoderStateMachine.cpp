#include "DecoderStateMachine.h"
#include <NmraDcc.h> // Use NmraDcc's definitions for clarity

// Simple CV register simulation
static uint8_t cv_register[256];

DecoderStateMachine::DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address)
    : _txManager(txManager), _type(type), _address(address), _last_addressed_time(0) {
        // Initialize some dummy CVs
        cv_register[8] = 151; // Manufacturer ID
    }

void DecoderStateMachine::handleDccPacket(const DCCMessage& dccMsg) {
    NmraDcc dccPacket;
    dccPacket.setPacket(dccMsg.getData(), dccMsg.getLength());

    bool for_us = false;
    if (dccPacket.getAddress() == _address) {
        for_us = true;
    }

    // Per RCN-217, decoders should also respond after any broadcast or other MOB packet
    if (_type == DecoderType::LOCOMOTIVE && dccPacket.isMobileDecoderPacket()) {
        for_us = true;
    }

    if (!for_us) return;

    _last_addressed_time = millis();

    if (_type == DecoderType::LOCOMOTIVE) {
        // Check for specific commands that require a specific response
        if (dccPacket.isPomReadCv()) {
            uint16_t cv = dccPacket.getPomCv();
            if (cv < 256) {
                _txManager.sendPomResponse(cv_register[cv]);
            }
        } else if (dccPacket.isPomWriteCv()) {
            // Per RCN-217, a POM write is just acknowledged by an address broadcast
            _txManager.sendAddress(_address);
        }
        else {
            // Default response for any other packet is address broadcast
            _txManager.sendAddress(_address);
        }
    }
    else if (_type == DecoderType::ACCESSORY) {
        // Default response is to send status
        _txManager.sendStatus1(0); // Dummy status
    }
}
