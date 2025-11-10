#include "RP2040RailcomHardware.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "RailcomProtocolDefs.h"

RP2040RailcomHardware::RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint rx_pin)
    : _uart(uart), _tx_pin(tx_pin), _rx_pin(rx_pin) {
}

void RP2040RailcomHardware::begin() {
    uart_init(_uart, UART_DCC_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RP2040RailcomHardware::end() {
    uart_deinit(_uart);
}

void RP2040RailcomHardware::on_cutout_start() {
    uart_set_baudrate(_uart, UART_RAILCOM_BAUDRATE);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);

    send_queued_messages();
    uart_tx_wait_blocking(_uart);

    uart_set_baudrate(_uart, UART_DCC_BAUDRATE);
}

void RP2040RailcomHardware::queue_message(uint8_t channel, const std::vector<uint8_t>& message) {
    if (channel == 1) _ch1_queue.push(message);
    else _ch2_queue.push(message);
}

void RP2040RailcomHardware::task() {
    // No-op, message sending is now triggered by on_cutout_start
}

void RP2040RailcomHardware::send_queued_messages() {
    if (!_ch1_queue.empty()) {
        const auto& msg = _ch1_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch1_queue.pop();
    }

    sleep_us(RAILCOM_CH2_DELAY_US);

    while (!_ch2_queue.empty()) {
        const auto& msg = _ch2_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch2_queue.pop();
    }
}

int RP2040RailcomHardware::available() {
    return uart_is_readable(_uart);
}

int RP2040RailcomHardware::read() {
    return uart_getc(_uart);
}
