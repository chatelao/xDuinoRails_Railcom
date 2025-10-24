#ifndef RAILCOM_RX_H
#define RAILCOM_RX_H

#include <Arduino.h>
#include <vector>
#include "hardware/uart.h"
#include "Railcom.h"

class RailcomRx {
public:
    RailcomRx(uart_inst_t* uart, uint rx_pin);
    void begin();
    void end();

    RailcomMessage* readMessage();

private:
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);
    RailcomMessage* parseMessage(const std::vector<uint8_t>& buffer);

    uart_inst_t* _uart;
    uint _rx_pin;
};

#endif // RAILCOM_RX_H
