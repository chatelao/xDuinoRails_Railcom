#ifndef RAILCOM_RX_H
#define RAILCOM_RX_H

#include <Arduino.h>
#include <vector>
#include "hardware/uart.h"

class RailcomRx {
public:
    RailcomRx(uart_inst_t* uart, uint rx_pin);
    void begin();
    void end();
    void set_decoder_address(uint16_t address);
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);

private:
    uart_inst_t* _uart;
    uint _rx_pin;
    uint16_t _decoder_address;
};

#endif // RAILCOM_RX_H
