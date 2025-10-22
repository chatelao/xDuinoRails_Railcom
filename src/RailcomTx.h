#ifndef RAILCOM_TX_H
#define RAILCOM_TX_H

#include <Arduino.h>
#include <vector>
#include <queue>
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "Railcom.h" // For DCCMessage

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

#endif // RAILCOM_TX_H
