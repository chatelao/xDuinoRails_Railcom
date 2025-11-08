#ifndef DECODER_STATE_MACHINE_H
#define DECODER_STATE_MACHINE_H

#include "Railcom.h"
#include "RailcomTx.h"
#include "RailcomRx.h"

#include "RailcomDccParser.h"

enum class DecoderType {
    LOCOMOTIVE,
    ACCESSORY,
    ACCESSORY_STANDARD,
    ACCESSORY_EXTENDED
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
    DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint8_t cv28, uint8_t cv29, uint16_t manufacturerId = 0, uint32_t productId = 0);

    // This is the main entry point. It analyzes the DCC packet and queues
    // the appropriate RailCom response.
    void handleDccPacket(const DCCMessage& dccMsg);

private:
    void setupCallbacks();

    RailcomTx& _txManager;
    DecoderType _type;
    uint16_t _address;
    uint8_t _cv28;
    uint8_t _cv29;
    uint16_t _manufacturerId;
    uint32_t _productId;

    // Internal state
    LogonState _logonState;
    RailcomDccParser _dccParser;
    unsigned long _last_addressed_time;
    // The current state of the accessory decoder's outputs.
    // This is a member variable to avoid state management bugs with multiple instances.
    uint8_t _accessory_state;
    // Flag to control the address broadcast on Channel 1.
    bool _channel1_broadcast_enabled;
};

#endif // DECODER_STATE_MACHINE_H
