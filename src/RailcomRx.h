/**
 * @file RailcomRx.h
 * @brief Manages the reception and parsing of RailCom messages.
 * @details This class provides a high-level interface for receiving raw
 *          RailCom byte streams from the hardware layer, decoding them, and
 *          parsing them into specific message structs defined in Railcom.h.
 */
#ifndef RAILCOM_RX_H
#define RAILCOM_RX_H

#include "Railcom.h"
#include "RailcomRxHardware.h"
#include <vector>

/**
 * @class RailcomRx
 * @brief Handles the reception, decoding, and parsing of RailCom messages.
 * @details This class reads raw byte data from the hardware, decodes the
 *          4-of-8 encoding, and parses the resulting datagrams into meaningful
 *          RailcomMessage structs. It manages state for multi-part messages
 *          like long addresses.
 */
class RailcomRx {
public:
    /**
     * @brief Constructs a RailcomRx object.
     * @param hardware A pointer to a RailcomHardware implementation for physical layer communication.
     */
    RailcomRx(RailcomRxHardware* hardware);

    /**
     * @brief Initializes the receiver and underlying hardware.
     */
    void begin();

    /**
     * @brief A task function that can be called periodically in a loop.
     * @details Currently, this is a placeholder and has no specific function.
     *          It is intended for future use, such as handling timeouts or
     *          other state management that requires periodic checks.
     */
    void task();

    /**
     * @brief Reads and parses an available RailCom message.
     * @details This is the primary method for retrieving messages. It reads raw
     *          bytes from the hardware, attempts to parse them into a message,
     *          and returns a pointer to a RailcomMessage struct if successful.
     *          The caller is responsible for casting the base pointer to the
     *          appropriate message type based on the message ID.
     * @return A pointer to a parsed RailcomMessage, or nullptr if no valid message was received.
     */
    RailcomMessage* read();

    /**
     * @brief Sets the decoder context to disambiguate messages with shared IDs.
     * @details Some message IDs (e.g., 3 and 4) have different meanings depending
     *          on whether the sender is a mobile or stationary decoder. This
     *          method allows the application to provide that context.
     * @param context The current decoder context (MOBILE or STATIONARY).
     */
    void setContext(DecoderContext context);

    /**
     * @brief Prints a human-readable representation of the last received message.
     * @details This method formats the data from the last successfully parsed
     *          message and outputs it to the provided Arduino Print stream.
     *          It also handles stateful printing, like calculating and displaying
     *          the effective address after both ADR_HIGH and ADR_LOW have been received.
     * @param stream A reference to a Print object (e.g., `Serial`).
     */
    void print(Print& stream);

    /**
     * @brief Puts the receiver into a state where it expects a Data Space message.
     * @details Data Space messages (RCN-218) do not have a standard RailCom ID and
     *          are sent as a direct response to a specific DCC command. This method
     *          flags the receiver to use a special parsing logic for the next
     *          incoming message.
     * @param dataSpaceNum The data space number that is expected, used for CRC calculation.
     */
    void expectDataSpaceResponse(uint8_t dataSpaceNum);

private:
    /**
     * @brief Reads raw bytes from the hardware buffer.
     * @param buffer A reference to a vector where the read bytes will be stored.
     * @param timeout_ms The timeout in milliseconds to wait for data.
     * @return True if bytes were read, false otherwise.
     */
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);

    /**
     * @brief Parses a buffer of raw, decoded bytes into a RailcomMessage struct.
     * @param buffer A const reference to the vector containing the decoded datagram.
     * @return A pointer to a newly allocated RailcomMessage struct, or nullptr on failure.
     */
    RailcomMessage* parseMessage(const std::vector<uint8_t>& buffer);

    RailcomRxHardware* _hardware; ///< Pointer to the hardware abstraction layer.
    std::vector<uint8_t> _lastRawBytes; ///< Stores the raw bytes of the last received message.
    RailcomMessage* _lastMessage = nullptr; ///< Pointer to the last successfully parsed message.
    uint8_t _lastAdrHigh = 0; ///< Stores the high byte of a long address for stateful address calculation.
    DecoderContext _context = DecoderContext::UNKNOWN; ///< The current context for parsing ambiguous messages.
    bool _is_data_space_expected = false; ///< Flag indicating that the next message should be a Data Space response.
    uint8_t _expected_data_space_num = 0; ///< The expected data space number for CRC calculation.
};

#endif // RAILCOM_RX_H
