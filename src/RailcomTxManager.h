#ifndef RAILCOM_TX_MANAGER_H
#define RAILCOM_TX_MANAGER_H

#include "Railcom.h"
#include "RailcomSender.h"

class RailcomTxManager {
public:
    RailcomTxManager(RailcomSender& sender);

    // --- Vehicle Decoder (MOB) Functions ---
    void sendPomResponse(uint8_t cvValue);
    void sendAddress(uint16_t address);
    void sendDynamicData(uint8_t subIndex, uint8_t value);
    void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]);
    void handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn);

    // --- Accessory Decoder (STAT) Functions ---
    void sendServiceRequest(uint16_t accessoryAddress, bool isExtended);
    void sendStatus1(uint8_t status);
    void sendStatus4(uint8_t status);
    void sendError(uint8_t errorCode);

    // --- RCN-218 DCC-A Functions ---
    void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId);
    void sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps);
    void sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum);
    void sendAck();
    void sendNack();

private:
    RailcomSender& _sender;
    bool _long_address_alternator;

    void sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits);
    void sendBundledDatagram(uint64_t payload);
};

#endif // RAILCOM_TX_MANAGER_H
