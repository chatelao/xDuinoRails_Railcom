#ifndef RP2040_RAILCOM_HARDWARE_H
#define RP2040_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include "hardware/uart.h"
#include <vector>

class RP2040RailcomHardware : public RailcomHardware {
public:
    RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint rx_pin);
    ~RP2040RailcomHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;

    void send_bytes(const std::vector<uint8_t>& bytes) override;

    int available() override;
    int read() override;

private:
    uart_inst_t* _uart;
    uint _tx_pin;
    uint _rx_pin;
};

#endif // RP2040_RAILCOM_HARDWARE_H
