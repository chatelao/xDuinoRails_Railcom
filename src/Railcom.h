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

#include "IRailcomStream.h"

class RailcomSender {
public:
    RailcomSender(IRailcomStream* stream, uint pio_pin);
    void begin();
    void end();
    void send_dcc_with_cutout(const DCCMessage& dccMsg);
    void queue_message(uint8_t channel, const std::vector<uint8_t>& message);
    void task();

private:
    friend void railcom_pio_irq_handler();
    void pio_init();
    void send_queued_messages();

    IRailcomStream* _stream;
    uint _pio_pin;
    PIO _pio;
    uint _sm;
    uint _offset; // PIO program offset

#ifdef AUNIT_H
public:
#endif
    std::queue<std::vector<uint8_t>> _ch1_queue;
    std::queue<std::vector<uint8_t>> _ch2_queue;
    volatile bool _send_pending;
};

class RailcomReceiver {
public:
    RailcomReceiver(IRailcomStream* stream);
    void begin();
    void end();
    void set_decoder_address(uint16_t address);

    // Reads raw bytes from the stream into the buffer
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);

private:
    IRailcomStream* _stream;
    uint16_t _decoder_address;
};

#endif // RAILCOM_H
