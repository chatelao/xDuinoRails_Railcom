/**
 * @file RailcomDccParser.h
 * @brief Parses DCC packets and triggers callbacks for specific commands.
 * @details This class is designed to decode raw DCC messages and invoke
 *          corresponding std::function callbacks when a relevant command is
 *          detected. This decouples the low-level DCC parsing from the high-level
 *          decoder logic. The user of this class (e.g., DecoderStateMachine)
 *          is responsible for defining the behavior by assigning lambda
 *          functions or other callables to the public callback members.
 */
#ifndef RAILCOM_DCC_PARSER_H
#define RAILCOM_DCC_PARSER_H

#include "Railcom.h"
#include <functional>

/**
 * @class RailcomDccParser
 * @brief A callback-based parser for DCC commands relevant to RailCom.
 */
class RailcomDccParser {
public:
    // --- Callbacks for RCN-218 Commands ---

    /**
     * @brief Called when a DCC-A Logon Enable command is received.
     * @param group The group ID from the command.
     * @param zid The ZID from the command.
     * @param sessionId The session ID from the command.
     * @see RCN-218, Chapter 3
     */
    std::function<void(uint8_t group, uint16_t zid, uint8_t sessionId)> onLogonEnable;

    /**
     * @brief Called when a DCC-A Select command is received.
     * @param manufacturerId The manufacturer ID to select.
     * @param productId The product ID to select.
     * @param subCmd The sub-command code.
     * @param data A pointer to the sub-command's data payload.
     * @param len The length of the data payload.
     * @see RCN-218, Chapter 3
     */
    std::function<void(uint16_t manufacturerId, uint32_t productId, uint8_t subCmd, const uint8_t* data, size_t len)> onSelect;

    /**
     * @brief Called when a DCC-A Logon Assign command is received.
     * @param manufacturerId The manufacturer ID of the target decoder.
     * @param productId The product ID of the target decoder.
     * @param address The new address to assign to the decoder.
     * @see RCN-218, Chapter 3
     */
    std::function<void(uint16_t manufacturerId, uint32_t productId, uint16_t address)> onLogonAssign;

    /** @brief Called for a GET_DATA_START command. @see RCN-218, 5.2 */
    std::function<void()> onGetDataStart;

    /** @brief Called for a GET_DATA_CONT command. @see RCN-218, 5.2 */
    std::function<void()> onGetDataCont;

    /** @brief Called for a SET_DATA command. @see RCN-218, 5.2 */
    std::function<void(const uint8_t*, size_t)> onSetData;

    /** @brief Called for a SET_DATA_END command. @see RCN-218, 5.2 */
    std::function<void()> onSetDataEnd;

    // --- Callbacks for RCN-217 Commands ---

    /**
     * @brief Called when a POM Read CV command is received.
     * @param cv The CV to read (1-1024).
     * @param address The address of the target decoder.
     * @see NMRA S-9.2.1
     */
    std::function<void(uint16_t cv, uint16_t address)> onPomReadCv;

    /**
     * @brief Called when a POM Write CV command is received.
     * @param cv The CV to write to (1-1024).
     * @param value The 8-bit value to write.
     * @param address The address of the target decoder.
     * @see NMRA S-9.2.1
     */
    std::function<void(uint16_t cv, uint8_t value, uint16_t address)> onPomWriteCv;

    /**
     * @brief Called when a POM Write Bit command is received.
     * @param cv The CV to modify (1-1024).
     * @param bit The bit number to write (0-7).
     * @param value The value of the bit (0 or 1).
     * @param address The address of the target decoder.
     * @see NMRA S-9.2.1
     */
    std::function<void(uint16_t cv, uint8_t bit, uint8_t value, uint16_t address)> onPomWriteBit;

    /**
     * @brief Called when an accessory decoder command is received.
     * @param address The address of the accessory.
     * @param activate True to activate the output, false to deactivate.
     * @param output The specific output on the accessory to control.
     * @see NMRA S-9.2.1
     */
    std::function<void(uint16_t address, bool activate, uint8_t output)> onAccessory;

    /**
     * @brief Called when a function group command is received (F0-F28).
     * @param address The address of the target decoder.
     * @param function The function number.
     * @param state The state of the function (true for on, false for off).
     */
    std::function<void(uint16_t address, uint8_t function, bool state)> onFunction;

    /**
     * @brief Called when an extended function command (XF) is received.
     * @param address The address of the target decoder.
     * @param command The extended command number (e.g., XF1, XF2).
     * @see RCN-217, 4.3.1
     */
    std::function<void(uint16_t address, uint8_t command)> onExtendedFunction;

    /**
     * @brief Called when a Data Space Read command is received.
     * @param address The address of the target decoder.
     * @param dataSpaceNum The number of the data space to read from.
     * @param startAddr The starting address within the data space.
     * @see RCN-218, 4.3
     */
    std::function<void(uint16_t address, uint8_t dataSpaceNum, uint8_t startAddr)> onDataSpaceRead;

    /**
     * @brief The main parsing function.
     * @details This method takes a DCCMessage, decodes it, and calls the
     *          appropriate registered callback, if any.
     * @param msg The DCCMessage to parse.
     * @param[out] response_sent A pointer to a boolean that will be set to true
     *             if a callback was invoked. This allows the caller to know if a
     *             specific RailCom response was triggered by this DCC packet.
     */
    void parse(const DCCMessage& msg, bool* response_sent = nullptr);
};

#endif // RAILCOM_DCC_PARSER_H
