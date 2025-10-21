#ifndef RAILCOM_H
#define RAILCOM_H

#include <Arduino.h>
#include <vector>
#include <queue>
#include "hardware/pio.h"
#include "hardware/uart.h"

// Forward declaration
class RailcomSender;

// Global instance pointer for the IRQ handler
extern RailcomSender* pio_sender_instance;

class DCCMessage {
public:
    DCCMessage();
    DCCMessage(const uint8_t* data, size_t len);
    const uint8_t* getData() const;
    size_t getLength() const;

private:
    uint8_t _data[12]; // Increased size to be safe
    size_t _len;
};

class RailcomSender {
public:
    RailcomSender(uart_inst_t* uart, uint txPin, uint rxPin);
    void begin();
    void end();

    // Sends a DCC packet and creates a cutout for RailCom messages
    void send_dcc_with_cutout(const DCCMessage& dccMsg);

    // Queues a raw, encoded message to be sent in the next available cutout
    void queue_message(uint8_t channel, const std::vector<uint8_t>& message);

    // This method should be called repeatedly in the main loop to handle sending.
    void task();

private:
    friend void railcom_pio_irq_handler();
    void pio_init();
    void send_queued_messages();

    uart_inst_t* _uart;
    uint _txPin;
    uint _rxPin;
    PIO _pio;
    uint _sm;

#ifdef AUNIT_H
public: // Make queues accessible for testing
#endif
    std::queue<std::vector<uint8_t>> _ch1_queue;
    std::queue<std::vector<uint8_t>> _ch2_queue;

    volatile bool _send_pending; // Flag set by ISR
};

class RailcomReceiver {
public:
    RailcomReceiver(uart_inst_t* uart, uint txPin, uint rxPin);
    void begin();
    void end();
    void set_decoder_address(uint16_t address);

    // Reads a complete RailCom response (multiple datagrams)
    bool read_response(std::vector<uint8_t>& buffer, uint timeout_ms);

private:
    uart_inst_t* _uart;
    uint _txPin;
    uint _rxPin;
    uint16_t _decoder_address;
};

#endif // RAILCOM_H
