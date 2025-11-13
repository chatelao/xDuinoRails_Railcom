/**
 * @file CommandStationRx.h
 * @brief Provides a high-level interface for a command station to receive and process RailCom messages.
 * @details This class simplifies the reception of RailCom data by managing message parsing
 *          and state, and providing callbacks for meaningful events like locomotive identification
 *          or accessory status changes. It is the receiving counterpart to the DecoderStateMachine.
 */
#ifndef COMMAND_STATION_RX_H
#define COMMAND_STATION_RX_H

#include "RailcomRx.h"
#include <functional>
#include <map>

/**
 * @class CommandStationRx
 * @brief High-level handler for receiving and interpreting RailCom messages.
 */
class CommandStationRx {
public:
    /**
     * @brief Callback function type for when a locomotive address is fully identified.
     * @param address The 14-bit address of the locomotive.
     */
    using LocoIdentifiedCallback = std::function<void(uint16_t address)>;

    /**
     * @brief Callback function type for when a POM response is received.
     * @param sourceAddress The address of the decoder that sent the response.
     * @param cvValue The value of the CV returned.
     */
    using PomResponseCallback = std::function<void(uint16_t sourceAddress, uint8_t cvValue)>;

    /**
     * @brief Callback function type for accessory decoder status changes.
     * @param accessoryAddress The address of the accessory decoder.
     * @param status The status byte reported by the decoder.
     */
    using AccessoryStatusCallback = std::function<void(uint16_t accessoryAddress, uint8_t status)>;


    /**
     * @brief Constructs a CommandStationRx object.
     * @param rx A pointer to a configured RailcomRx instance.
     */
    CommandStationRx(RailcomRx* rx);

    /**
     * @brief Initializes the command station receiver.
     */
    void begin();

    /**
     * @brief This method should be called repeatedly in the main loop.
     * @details It reads from the underlying RailcomRx, processes messages,
     *          manages state, and triggers callbacks.
     */
    void task();

    // --- Callback Attachment Methods ---

    /**
     * @brief Attaches a callback function to be called when a locomotive is identified.
     * @param callback The function to call.
     */
    void onLocoIdentified(LocoIdentifiedCallback callback);

    /**
     * @brief Attaches a callback function for POM responses.
     * @param callback The function to call.
     */
    void onPomResponse(PomResponseCallback callback);

    /**
     * @brief Attaches a callback function for accessory status updates.
     * @param callback The function to call.
     */
    void onAccessoryStatus(AccessoryStatusCallback callback);

private:
    RailcomRx* _rx; ///< Pointer to the low-level RailCom receiver.

    // --- State Management ---
    uint16_t _last_known_address; ///< Stores the last fully identified address.
    uint8_t _pending_adr_high;    ///< Stores the high byte of a long address while waiting for the low byte.

    // --- Callbacks ---
    LocoIdentifiedCallback _locoIdentifiedCallback;
    PomResponseCallback _pomResponseCallback;
    AccessoryStatusCallback _accessoryStatusCallback;

    /**
     * @brief Processes a single RailCom message.
     * @param msg A pointer to the message to process.
     */
    void processMessage(RailcomMessage* msg);
};

#endif // COMMAND_STATION_RX_H
