#ifndef DECODER_STATE_MACHINE_H
#define DECODER_STATE_MACHINE_H

#include "Railcom.h"
#include "RailcomTxManager.h"
#include "RailcomDccParser.h"

enum class DecoderType {
    LOCOMOTIVE,
    ACCESSORY
};

enum class LogonState {
    IDLE,
    WAITING_FOR_LOGON,
    IN_SINGULATION,
    ANNOUNCED,
    REGISTERED
};

class DecoderStateMachine {
public:
    DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address, uint16_t manufacturerId = 0, uint32_t productId = 0);

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
