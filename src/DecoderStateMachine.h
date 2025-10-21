#ifndef DECODER_STATE_MACHINE_H
#define DECODER_STATE_MACHINE_H

#include "Railcom.h"
#include "RailcomTxManager.h"

enum class DecoderType {
    LOCOMOTIVE,
    ACCESSORY
};

class DecoderStateMachine {
public:
    DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address);

    // This is the main entry point. It analyzes the DCC packet and queues
    // the appropriate RailCom response.
    void handleDccPacket(const DCCMessage& dccMsg);

private:
    RailcomTxManager& _txManager;
    DecoderType _type;
    uint16_t _address;

    // Internal state
    unsigned long _last_addressed_time;
};

#endif // DECODER_STATE_MACHINE_H
