#ifndef RAILCOM_TX_H
#define RAILCOM_TX_H

#include "Railcom.h"
#include "RailcomHardware.h"
#include <vector>

class RailcomTx {
public:
    RailcomTx(RailcomHardware* hardware);
    void begin();
    void end();
    void task();

    void send_dcc_with_cutout(const DCCMessage& dccMsg);

    // --- Vehicle Decoder (MOB) Functions ---
    void sendPomResponse(uint8_t cvValue);
    void sendExt(uint8_t type, uint8_t position);
    virtual void sendAddress(uint16_t address);
    void enableInfo1(const Info1Message& info1);
    void disableInfo1();
    void sendDynamicData(uint8_t subIndex, uint8_t value);
    void sendCvAuto(uint32_t cvAddress, uint8_t cvValue);
    void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4]);
    void handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn);

    // --- Accessory Decoder (STAT) Functions ---
    void sendServiceRequest(uint16_t accessoryAddress, bool isExtended);
    virtual void sendStatus1(uint8_t status);
    void sendStatus2(uint8_t status);
    void sendStatus4(uint8_t status);
    void sendTime(uint8_t timeValue, bool unit_is_second);
    void sendError(uint8_t errorCode);

    // --- RCN-218 DCC-A Functions ---
    virtual void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId);
    virtual void sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps);
    void sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum);
    void sendAck();
    void sendNack();

private:
    void sendDatagram(uint8_t channel, RailcomID id, uint64_t payload, uint8_t payloadBits);
    uint8_t buildInfo1Payload(const Info1Message& info1);

    RailcomHardware* _hardware;
    uint8_t _address_alternator; // 0 for ADR_HIGH, 1 for ADR_LOW, 2 for INFO1
    bool _info1_enabled;
    uint8_t _info1_payload;
};

#endif // RAILCOM_TX_H
