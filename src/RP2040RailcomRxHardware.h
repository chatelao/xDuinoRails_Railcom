#ifndef RP2040_RAILCOM_RX_HARDWARE_H
#define RP2040_RAILCOM_RX_HARDWARE_H

#include "RailcomRxHardware.h"
#include "hardware/uart.h"
#include <vector>

class RP2040RailcomRxHardware : public RailcomRxHardware {
public:
    RP2040RailcomRxHardware(uart_inst_t* uart, uint rx_pin);
    ~RP2040RailcomRxHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;

    int available() override;
    int read() override;

private:
    uart_inst_t* _uart;
    uint _rx_pin;
};

#endif // RP2040_RAILCOM_RX_HARDWARE_H
