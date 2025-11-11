/**
 * @file RP2040RailcomTxHardware.cpp
 * @brief Implementation of the RP2040RailcomTxHardware class.
 */
#include "RP2040RailcomTxHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

/**
 * @brief Constructs the hardware object.
 * @param uart Pointer to the RP2040 UART instance (e.g., `uart0`).
 * @param tx_pin The GPIO pin for UART TX.
 */
RP2040RailcomTxHardware::RP2040RailcomTxHardware(uart_inst_t* uart, uint tx_pin)
    : _uart(uart), _tx_pin(tx_pin) {
}

/**
 * @brief Initializes the UART hardware for transmission.
 */
void RP2040RailcomTxHardware::begin() {
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
}

/**
 * @brief Deinitializes the UART hardware.
 */
void RP2040RailcomTxHardware::end() {
    uart_deinit(_uart);
}

/**
 * @brief Periodic task function. Currently empty.
 */
void RP2040RailcomTxHardware::task() {
    // This is intentionally left empty.
}

/**
 * @brief Sends a block of bytes over the UART.
 */
void RP2040RailcomTxHardware::send_bytes(const std::vector<uint8_t>& bytes) {
    uart_write_blocking(_uart, bytes.data(), bytes.size());
    uart_tx_wait_blocking(_uart);
}
