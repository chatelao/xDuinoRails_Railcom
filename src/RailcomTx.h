/**
 * @file RailcomTx.h
 * @brief Manages the transmission of RailCom messages.
 * @details This class provides a high-level interface for sending various
 *          RailCom messages as specified in RCN-217 and RCN-218. It handles
 *          message queuing for both RailCom channels and interacts with the
 *          hardware abstraction layer to transmit the final encoded datagrams.
 */
#ifndef RAILCOM_TX_H
#define RAILCOM_TX_H

#include "Railcom.h"
#include "RailcomTxHardware.h"
#include <vector>
#include <queue>

/**
 * @class RailcomTx
 * @brief Handles the queuing and transmission of RailCom messages.
 * @details This class is responsible for sending all types of RailCom messages.
 *          It queues messages for Channel 1 (address broadcast) and Channel 2
 *          (data), and sends them when the `on_cutout_start` method is called,
 *          simulating the DCC cutout period.
 */
class RailcomTx {
public:
    /**
     * @brief Constructs a RailcomTx object.
     * @param hardware A pointer to a RailcomTxHardware implementation for physical layer communication.
     */
    RailcomTx(RailcomTxHardware* hardware);

    /**
     * @brief Initializes the transmitter and underlying hardware.
     */
    void begin();

    /**
     * @brief Deinitializes the transmitter and underlying hardware.
     */
    void end();

    /**
     * @brief Called by the application to signal the start of a DCC cutout.
     * @details This method triggers the transmission of any queued messages.
     *          It sends a Channel 1 message, followed by a Channel 2 message
     *          if one is available in the queue.
     * @param elapsed_us The time in microseconds since the last cutout, used for timing.
     */
    void on_cutout_start(uint32_t elapsed_us = 0);

    // --- Vehicle Decoder (MOB) Functions ---

    /**
     * @brief Sends a POM (Program on Main) response on Channel 2.
     * @details This is typically sent in response to a DCC POM read command.
     * @param cvValue The value of the CV that was read.
     * @see RCN-217, 5.2.1
     */
    void sendPomResponse(uint8_t cvValue);

    /**
     * @brief Sends a driving information message (ID 4) on Channel 2.
     * @param speed The current speed of the vehicle.
     * @param motorLoad The current motor load.
     * @param statusFlags Additional status flags.
     * @see RCN-217, 5.2.5
     */
    void sendInfo(uint16_t speed, uint8_t motorLoad, uint8_t statusFlags);

    /**
     * @brief Sends an extended message (ID 3) on Channel 2.
     * @param type The message type.
     * @param position The position value.
     * @see RCN-217, 5.2.4
     */
    void sendExt(uint8_t type, uint8_t position);

    /**
     * @brief Sends the decoder's address on Channel 1.
     * @details This method handles both short and long addresses. For long addresses,
     *          it alternates between sending ADR_HIGH and ADR_LOW messages.
     *          It can also be configured to send INFO1 messages in the broadcast cycle.
     * @param address The decoder's 14-bit address.
     * @see RCN-217, 5.2.2 & 5.2.3
     */
    virtual void sendAddress(uint16_t address);

    /**
     * @brief Enables the transmission of INFO1 messages in the Channel 1 broadcast cycle.
     * @param info1 The INFO1 message data to be broadcast.
     * @see RCN-217, 5.2.4
     */
    void enableInfo1(const Info1Message& info1);

    /**
     * @brief Disables the transmission of INFO1 messages in the Channel 1 broadcast cycle.
     */
    void disableInfo1();

    /**
     * @brief Sends a dynamic data message (ID 7) on Channel 2.
     * @param subIndex The sub-index of the data.
     * @param value The value of the data.
     * @see RCN-217, 5.2.8
     */
    void sendDynamicData(uint8_t subIndex, uint8_t value);

    /**
     * @brief Sends an automatic CV broadcast message (ID 12) on Channel 2.
     * @param cvAddress The address of the CV.
     * @param cvValue The value of the CV.
     * @see RCN-217, 5.2.11
     */
    void sendCvAuto(uint32_t cvAddress, uint8_t cvValue);

    /**
     * @brief Sends an extended POM response (IDs 8-11) on Channel 2.
     * @param sequence The sequence number (0-3) of the XPOM message.
     * @param cvValues A pointer to an array of 4 CV values.
     * @see RCN-217, 5.2.9
     */
    void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]);

    /**
     * @brief Handles the rerailing search procedure by sending a RERAIL message (ID 14).
     * @param address The decoder address.
     * @param secondsSincePowerOn The number of seconds since the decoder was powered on.
     * @see RCN-217, 5.2.12
     */
    void handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn);

    /**
     * @brief Sends a block message (ID 13) on Channel 2, used during logon.
     * @param data The 24-bit block data.
     * @see RCN-218, 4.1
     */
    void sendBlock(uint32_t data);

    // --- Accessory Decoder (STAT) Functions ---

    /**
     * @brief Sends a service request (ID 14) on Channel 2.
     * @param accessoryAddress The address of the accessory decoder.
     * @param isExtended True if the address is an extended accessory address.
     * @see RCN-217, 5.2.12
     */
    void sendServiceRequest(uint16_t accessoryAddress, bool isExtended);

    /**
     * @brief Sends a status message for 1 turnout (ID 4) on Channel 2.
     * @param status The status byte.
     * @see RCN-217, 5.2.5
     */
    virtual void sendStatus1(uint8_t status);

    /**
     * @brief Sends a status message for 2 turnouts (ID 8) on Channel 2.
     * @param status The status byte.
     * @see RCN-217, 5.2.9
     */
    void sendStatus2(uint8_t status);

    /**
     * @brief Sends a status message for 4 turnouts (ID 3) on Channel 2.
     * @param status The status byte.
     * @see RCN-217, 5.2.4
     */
    void sendStatus4(uint8_t status);

    /**
     * @brief Sends a time message (ID 5) on Channel 2.
     * @param timeValue The time value (0-127).
     * @param unit_is_second True for seconds, false for 0.1 seconds.
     * @see RCN-217, 5.2.6
     */
    void sendTime(uint8_t timeValue, bool unit_is_second);

    /**
     * @brief Sends an error message (ID 6) on Channel 2.
     * @param errorCode The error code.
     * @see RCN-217, 5.2.7
     */
    void sendError(uint8_t errorCode);

    // --- RCN-218 DCC-A Functions ---

    /**
     * @brief Sends the decoder's unique ID (ID 15) on Channel 2.
     * @param manufacturerId The 16-bit manufacturer ID.
     * @param productId The 32-bit unique product ID.
     * @see RCN-218, 4.3
     */
    virtual void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId);

    /**
     * @brief Sends the decoder's state (ID 13) on Channel 2.
     * @param changeFlags Flags indicating changes in the decoder state.
     * @param changeCount A counter that increments on each state change.
     * @param protocolCaps Flags indicating the decoder's protocol capabilities.
     * @see RCN-218, 4.2
     */
    virtual void sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps);

    /**
     * @brief Sends a Data Space message on Channel 2.
     * @details This is a special message format without a standard RailCom ID.
     * @param data A pointer to the data payload.
     * @param len The length of the data payload.
     * @param dataSpaceNum The data space number this message belongs to.
     * @see RCN-218, 5.2
     */
    void sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum);

    /**
     * @brief Sends an ACK (Acknowledge) signal on Channel 2.
     * @see RCN-217, 3.4.1
     */
    void sendAck();

    /**
     * @brief Sends a NACK (Not Acknowledge) signal on Channel 2.
     * @see RCN-218, 5.2
     */
    void sendNack();

private:
    /**
     * @brief Encodes and queues a datagram for transmission.
     * @param channel The channel (1 or 2) to send the message on.
     * @param id The RailcomID of the message.
     * @param payload The message payload data.
     * @param payloadBits The number of bits in the payload.
     */
    void sendDatagram(uint8_t channel, RailcomID id, uint64_t payload, uint8_t payloadBits);

    /**
     * @brief Builds the 8-bit payload for an INFO1 message from its struct.
     * @param info1 The Info1Message struct.
     * @return The calculated 8-bit payload.
     */
    uint8_t buildInfo1Payload(const Info1Message& info1);

    RailcomTxHardware* _hardware; ///< Pointer to the hardware abstraction layer.
    uint8_t _address_alternator; ///< State machine for alternating ADR_HIGH, ADR_LOW, INFO1.
    bool _info1_enabled;         ///< Flag to enable/disable INFO1 broadcast.
    uint8_t _info1_payload;      ///< Cached payload for INFO1 messages.

    std::queue<std::vector<uint8_t>> _ch1_queue; ///< Queue for Channel 1 messages.
    std::queue<std::vector<uint8_t>> _ch2_queue; ///< Queue for Channel 2 messages.
};

#endif // RAILCOM_TX_H
