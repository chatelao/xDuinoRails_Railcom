/**
 * @file DecoderStateMachine.h
 * @brief Manages the state and logic of a RailCom-enabled decoder.
 * @details This class acts as the central "brain" for a decoder, listening for
 *          DCC commands via the `RailcomDccParser` and sending the appropriate
 *          RailCom responses via the `RailcomTx` manager. It encapsulates the
 *          logic for features like the RailComPlus logon procedure (RCN-218)
 *          and automatic CV broadcasting (RCN-217).
 */
#ifndef DECODER_STATE_MACHINE_H
#define DECODER_STATE_MACHINE_H

#include "Railcom.h"
#include "RailcomTx.h"
#include "RailcomRx.h"

#include "RailcomDccParser.h"
#include <map>
#include <vector>
#include <iterator>

/**
 * @enum DecoderType
 * @brief Defines the type of the decoder.
 */
enum class DecoderType {
    LOCOMOTIVE,         ///< A mobile decoder in a locomotive.
    ACCESSORY,          ///< A generic stationary accessory decoder.
    ACCESSORY_STANDARD, ///< A stationary decoder with a standard address.
    ACCESSORY_EXTENDED  ///< A stationary decoder with an extended address.
};

/**
 * @enum LogonState
 * @brief Defines the states for the RCN-218 RailComPlus automatic logon procedure.
 * @see RCN-218, Chapter 3
 */
enum class LogonState {
    IDLE,               ///< Not currently participating in logon.
    WAITING_FOR_LOGON,  ///< Logon has been enabled, waiting for a SELECT command.
    IN_SINGULATION,     ///< In the backoff/singulation phase.
    ANNOUNCED,          ///< The decoder has announced its unique ID.
    REGISTERED          ///< The command station has assigned a slot to the decoder.
};

/**
 * @class DecoderStateMachine
 * @brief Encapsulates the complete logic for a RailCom-enabled decoder.
 * @details This class connects the DCC parsing logic with the RailCom transmission
 *          logic. It maintains the internal state of the decoder, including its
 *          address, configuration variables (CVs), and its current state within
 *          the RailComPlus logon procedure. The primary entry point is
 *          `handleDccPacket`, which should be called for every incoming DCC message.
 */
class DecoderStateMachine {
public:
    /**
     * @brief Constructs a DecoderStateMachine.
     * @param txManager A reference to the RailcomTx object for sending messages.
     * @param type The type of the decoder (e.g., LOCOMOTIVE).
     * @param address The primary address of the decoder.
     * @param cv28 Configuration Variable 28.
     * @param cv29 Configuration Variable 29 (controls RailCom enable state).
     * @param manufacturerId The manufacturer ID of the decoder (for RCN-218).
     * @param productId The unique product ID of the decoder (for RCN-218).
     */
    DecoderStateMachine(RailcomTx& txManager, DecoderType type, uint16_t address, uint8_t cv28, uint8_t cv29, uint16_t manufacturerId = 0, uint32_t productId = 0);

    /**
     * @brief Main entry point for processing DCC packets.
     * @details This method analyzes the incoming DCC packet and triggers the
     *          appropriate RailCom response or state change. It should be
     *          called for every DCC packet received by the decoder.
     * @param dccMsg The DCCMessage object to be processed.
     */
    void handleDccPacket(const DCCMessage& dccMsg);

    /**
     * @brief Periodic task function.
     * @details This should be called periodically in the main application loop
     *          to handle ongoing background tasks, such as broadcasting the
     *          next CV in the automatic CV broadcast cycle.
     */
    void task();

private:
    /**
     * @brief Sets up the callbacks for the internal RailcomDccParser.
     */
    void setupCallbacks();

    RailcomTx& _txManager;      ///< Reference to the transmitter.
    DecoderType _type;          ///< The type of this decoder.
    uint16_t _address;          ///< The primary address.
    uint8_t _cv28;              ///< CV28 value.
    uint8_t _cv29;              ///< CV29 value.
    uint16_t _manufacturerId;   ///< Manufacturer ID for RCN-218.
    uint32_t _productId;        ///< Product ID for RCN-218.

    // --- Internal State ---
    LogonState _logonState;             ///< Current state in the RCN-218 logon process.
    RailcomDccParser _dccParser;        ///< The internal DCC parser instance.
    unsigned long _last_addressed_time; ///< Timestamp of the last DCC message addressed to this decoder.
    uint8_t _accessory_state;           ///< The current state of the accessory decoder's outputs.
    bool _channel1_broadcast_enabled;   ///< Flag to control the address broadcast on Channel 1.

    // --- RCN-218 Backoff State ---
    uint8_t _backoff_counter;   ///< The counter for the backoff algorithm.
    uint8_t _backoff_value;     ///< The current value to count down from in the backoff algorithm.

    // --- CV-Auto Broadcast State ---
    std::map<uint32_t, uint8_t> _cvs;                                ///< Map to store CVs for the automatic broadcast.
    std::map<uint32_t, uint8_t>::iterator _cv_auto_iterator;         ///< Iterator to track the next CV to send.
    bool _cv_auto_broadcast_active;                                 ///< Flag indicating if the broadcast is active.

    // --- RCN-218 Data Space Storage ---
    std::map<uint8_t, std::vector<uint8_t>> _data_spaces; ///< Map to store data for various data spaces.
};

#endif // DECODER_STATE_MACHINE_H
