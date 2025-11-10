#include "RP2040RailcomRxHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

RP2040RailcomRxHardware::RP2040RailcomRxHardware(uart_inst_t* uart, uint rx_pin)
    : _uart(uart), _rx_pin(rx_pin) {
}

void RP2040RailcomRxHardware::begin() {
    // Initialize UART for RailCom communication baud rate
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RP2040RailcomRxHardware::end() {
    uart_deinit(_uart);
}

void RP2040RailcomRxHardware::task() {
    // No specific task needed for RX hardware.
}

int RP2040RailcomRxHardware::available() {
    return uart_is_readable(_uart);
}

int RP2040RailcomRxHardware::read() {
    if (uart_is_readable(_uart)) {
        return uart_getc(_uart);
    }
    return -1;
}
