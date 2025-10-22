#ifndef RAILCOM_TX_H
#define RAILCOM_TX_H

#include <Arduino.h>
#include <vector>
#include <queue>
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "Railcom.h" // For DCCMessage
#include "RailcomEncoding.h"

class RailcomTx;
extern RailcomTx* pio_sender_instance;

class RailcomTx {
public:
    RailcomTx(uart_inst_t* uart, uint tx_pin, uint pio_pin);
    void begin();
    void end();
    void send_dcc_with_cutout(const DCCMessage& dccMsg);
    void queue_message(uint8_t channel, const std::vector<uint8_t>& message);
    void task();

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
    friend void railcom_pio_irq_handler();
    void pio_init();
    void send_queued_messages();

    uart_inst_t* _uart;
    uint _tx_pin;
    uint _pio_pin;
    PIO _pio;
    uint _sm;
    uint _offset;

#ifdef AUNIT_H
public:
#endif
    std::queue<std::vector<uint8_t>> _ch1_queue;
    std::queue<std::vector<uint8_t>> _ch2_queue;
    volatile bool _send_pending;

    bool _long_address_alternator;
    void sendDatagram(uint8_t channel, RailcomID id, uint32_t payload, uint8_t payloadBits);
    void sendBundledDatagram(uint64_t payload);
};

#endif // RAILCOM_TX_H
