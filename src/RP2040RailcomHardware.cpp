#include "RP2040RailcomHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

RP2040RailcomHardware::RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint rx_pin)
    : _uart(uart), _tx_pin(tx_pin), _rx_pin(rx_pin) {
}

void RP2040RailcomHardware::begin() {
    // Initialize UART for RailCom communication baud rate
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RP2040RailcomHardware::end() {
    uart_deinit(_uart);
}

void RP2040RailcomHardware::task() {
    // No longer responsible for sending logic, this can be handled elsewhere if needed.
}

void RP2040RailcomHardware::send_bytes(const std::vector<uint8_t>& bytes) {
    uart_write_blocking(_uart, bytes.data(), bytes.size());
    uart_tx_wait_blocking(_uart);
}

int RP2040RailcomHardware::available() {
    return uart_is_readable(_uart);
}

int RP2040RailcomHardware::read() {
    if (uart_is_readable(_uart)) {
        return uart_getc(_uart);
    }
    return -1;
}
