#include "RailcomReceiver.h"

RailcomReceiver::RailcomReceiver(uart_inst_t* uart, uint rx_pin)
    : _uart(uart), _rx_pin(rx_pin), _decoder_address(0) {}

void RailcomReceiver::begin() {
    uart_init(_uart, 250000);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RailcomReceiver::end() {
    uart_deinit(_uart);
}

void RailcomReceiver::set_decoder_address(uint16_t address) {
    _decoder_address = address;
}

bool RailcomReceiver::read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) {
    buffer.clear();
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        if (uart_is_readable(_uart)) {
            buffer.push_back(uart_getc(_uart));
        } else if (!buffer.empty()) {
            return true;
        }
    }
    return !buffer.empty();
}
