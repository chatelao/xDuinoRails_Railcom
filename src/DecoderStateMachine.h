#ifndef DECODER_STATE_MACHINE_H
#define DECODER_STATE_MACHINE_H

#include "Railcom.h"
#include "RailcomTx.h"
#include "RailcomRx.h"

#include "RailcomDccParser.h"
#include <map>
#include <iterator>

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

    // Should be called periodically in the main loop to handle ongoing tasks
    // like the CV-Auto broadcast.
    void task();

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

    // --- CV-Auto Broadcast State ---
    // A map to store the CVs for the CV-Auto broadcast.
    std::map<uint32_t, uint8_t> _cvs;
    // An iterator to keep track of the next CV to send.
    std::map<uint32_t, uint8_t>::iterator _cv_auto_iterator;
    // A flag to indicate if the CV-Auto broadcast is active.
    bool _cv_auto_broadcast_active;
};

#endif // DECODER_STATE_MACHINE_H
