#ifndef RAILCOM_H
#define RAILCOM_H

#include <Arduino.h>
#include <vector>
#include <queue>
#include "hardware/pio.h"
#include "hardware/uart.h"

class RailcomSender;
extern RailcomSender* pio_sender_instance;

class DCCMessage {
public:
    DCCMessage();
    DCCMessage(const uint8_t* data, size_t len);
    const uint8_t* getData() const;
    size_t getLength() const;
private:
    uint8_t _data[12];
    size_t _len;
};

class RailcomSender {
public:
    RailcomSender(uart_inst_t* uart, uint tx_pin, uint pio_pin);
    void begin();
    void end();
    void send_dcc_with_cutout(const DCCMessage& dccMsg);
    void queue_message(uint8_t channel, const std::vector<uint8_t>& message);
    void task();

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
};

class RailcomReceiver {
public:
    RailcomReceiver(uart_inst_t* uart, uint rx_pin);
    void begin();
    void end();
    void set_decoder_address(uint16_t address);
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);

private:
    uart_inst_t* _uart;
    uint _rx_pin;
    uint16_t _decoder_address;
};

#endif // RAILCOM_H
