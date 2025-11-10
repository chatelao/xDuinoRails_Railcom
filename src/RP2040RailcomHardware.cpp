/**
 * @file RP2040RailcomHardware.cpp
 * @brief Implementation of the RP2040RailcomHardware class.
 */
#include "RP2040RailcomHardware.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

/**
 * @brief Constructs the hardware object.
 * @param uart Pointer to the RP2040 UART instance (e.g., `uart0`).
 * @param tx_pin The GPIO pin for UART TX.
 * @param rx_pin The GPIO pin for UART RX.
 */
RP2040RailcomHardware::RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint rx_pin)
    : _uart(uart), _tx_pin(tx_pin), _rx_pin(rx_pin) {
}

/**
 * @brief Initializes the UART hardware.
 * @details Sets the baud rate to the RailCom standard 250,000 and configures
 *          the specified GPIO pins for UART function.
 */
void RP2040RailcomHardware::begin() {
    uart_init(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

/**
 * @brief Deinitializes the UART hardware.
 */
void RP2040RailcomHardware::end() {
    uart_deinit(_uart);
}

/**
 * @brief Periodic task function. Currently empty.
 */
void RP2040RailcomHardware::task() {
    // This is intentionally left empty.
}

/**
 * @brief Sends a block of bytes over the UART.
 * @details This is a blocking call that waits for the entire buffer to be
 *          written to the UART FIFO and for the transmission to complete.
 * @param bytes The vector of bytes to send.
 */
void RP2040RailcomHardware::send_bytes(const std::vector<uint8_t>& bytes) {
    uart_write_blocking(_uart, bytes.data(), bytes.size());
    uart_tx_wait_blocking(_uart);
}

/**
 * @brief Checks for available data on the UART.
 * @return 1 if data is available, 0 otherwise.
 */
int RP2040RailcomHardware::available() {
    return uart_is_readable(_uart);
}

/**
 * @brief Reads a single byte from the UART.
 * @return The received byte, or -1 if no data is available.
 */
int RP2040RailcomHardware::read() {
    if (uart_is_readable(_uart)) {
        return uart_getc(_uart);
    }
    return -1;
}
