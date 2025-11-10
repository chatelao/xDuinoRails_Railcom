#include "RP2040RailcomTxHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

RP2040RailcomTxHardware::RP2040RailcomTxHardware(uart_inst_t* uart, uint tx_pin)
    : _uart(uart), _tx_pin(tx_pin) {
}

void RP2040RailcomTxHardware::begin() {
    // Initialize UART for RailCom communication baud rate
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
}

void RP2040RailcomTxHardware::end() {
    uart_deinit(_uart);
}

void RP2040RailcomTxHardware::task() {
    // No specific task needed for TX hardware.
}

void RP2040RailcomTxHardware::send_bytes(const std::vector<uint8_t>& bytes) {
    uart_write_blocking(_uart, bytes.data(), bytes.size());
    uart_tx_wait_blocking(_uart);
}
