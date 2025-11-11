/**
 * @file RP2040RailcomRxHardware.cpp
 * @brief Implementation of the RP2040RailcomRxHardware class.
 */
#include "RP2040RailcomRxHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

/**
 * @brief Constructs the hardware object.
 * @param uart Pointer to the RP2040 UART instance (e.g., `uart0`).
 * @param rx_pin The GPIO pin for UART RX.
 */
RP2040RailcomRxHardware::RP2040RailcomRxHardware(uart_inst_t* uart, uint rx_pin)
    : _uart(uart), _rx_pin(rx_pin) {
}

/**
 * @brief Initializes the UART hardware for reception.
 */
void RP2040RailcomRxHardware::begin() {
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

/**
 * @brief Deinitializes the UART hardware.
 */
void RP2040RailcomRxHardware::end() {
    uart_deinit(_uart);
}

/**
 * @brief Periodic task function. Currently empty.
 */
void RP2040RailcomRxHardware::task() {
    // This is intentionally left empty.
}

/**
 * @brief Checks for available data on the UART.
 */
int RP2040RailcomRxHardware::available() {
    return uart_is_readable(_uart);
}

/**
 * @brief Reads a single byte from the UART.
 */
int RP2040RailcomRxHardware::read() {
    if (uart_is_readable(_uart)) {
        return uart_getc(_uart);
    }
    return -1;
}
