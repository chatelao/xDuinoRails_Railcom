#ifndef RP2040_RAILCOM_TX_HARDWARE_H
#define RP2040_RAILCOM_TX_HARDWARE_H

#include "RailcomTxHardware.h"
#include "hardware/uart.h"
#include <vector>

class RP2040RailcomTxHardware : public RailcomTxHardware {
public:
    RP2040RailcomTxHardware(uart_inst_t* uart, uint tx_pin);
    ~RP2040RailcomTxHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;

    void send_bytes(const std::vector<uint8_t>& bytes) override;

private:
    uart_inst_t* _uart;
    uint _tx_pin;
};

#endif // RP2040_RAILCOM_TX_HARDWARE_H
