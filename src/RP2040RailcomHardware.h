#ifndef RP2040_RAILCOM_HARDWARE_H
#define RP2040_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include <queue>
#include <vector>

class RP2040RailcomHardware : public RailcomHardware {
public:
    RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint pio_pin, uint rx_pin);
    ~RP2040RailcomHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;

    void send_dcc_with_cutout(const DCCMessage& dccMsg) override;

    void queue_message(uint8_t channel, const std::vector<uint8_t>& message) override;

    int available() override;
    int read() override;

private:
    friend void railcom_pio_irq_handler_instance();

    void pio_init();
    void send_queued_messages();

    uart_inst_t* _uart;
    uint _tx_pin;
    uint _pio_pin;
    uint _rx_pin;
    PIO _pio;
    uint _sm;
    uint _offset;

    std::queue<std::vector<uint8_t>> _ch1_queue;
    std::queue<std::vector<uint8_t>> _ch2_queue;
    volatile bool _send_pending;
};

#endif // RP2040_RAILCOM_HARDWARE_H
