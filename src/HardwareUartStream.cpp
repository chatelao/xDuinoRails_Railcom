#include "HardwareUartStream.h"
#include "pico/stdlib.h"

HardwareUartStream::HardwareUartStream(uart_inst_t* uart, uint txPin, uint rxPin)
    : _uart(uart), _txPin(txPin), _rxPin(rxPin) {}

void HardwareUartStream::begin(uint32_t baudrate) {
    uart_init(_uart, baudrate);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
    gpio_set_function(_rxPin, GPIO_FUNC_UART);
}

void HardwareUartStream::end() {
    uart_deinit(_uart);
}

int HardwareUartStream::available() {
    return uart_is_readable(_uart) ? 1 : 0; // uart_is_readable doesn't give a count
}

int HardwareUartStream::read() {
    if (uart_is_readable(_uart)) {
        return uart_getc(_uart);
    }
    return -1;
}

void HardwareUartStream::write(const uint8_t* data, size_t len) {
    uart_write_blocking(_uart, data, len);
}

void HardwareUartStream::setBaudrate(uint32_t baudrate) {
    uart_set_baudrate(_uart, baudrate);
}

void HardwareUartStream::tx_wait_blocking() {
    uart_tx_wait_blocking(_uart);
}
