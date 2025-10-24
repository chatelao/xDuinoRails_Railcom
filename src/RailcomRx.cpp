#include "RailcomRx.h"
#include <cstring>

RailcomRx::RailcomRx(uart_inst_t* uart, uint rx_pin)
    : _uart(uart), _rx_pin(rx_pin) {}

void RailcomRx::begin() {
    uart_init(_uart, 250000);
    gpio_set_function(_rx_pin, GPIO_FUNC_UART);
}

void RailcomRx::end() {
    uart_deinit(_uart);
}

bool RailcomRx::read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) {
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

RailcomMessage* RailcomRx::readMessage() {
    std::vector<uint8_t> buffer;
    if (read_raw_bytes(buffer, 50)) {
        return parseMessage(buffer);
    }
    return nullptr;
}

RailcomMessage* RailcomRx::parseMessage(const std::vector<uint8_t>& buffer) {
    // This is a placeholder implementation. A real implementation would
    // need to decode the 4-of-8 encoding and parse the message format.
    if (buffer.size() >= 2) {
        PomMessage* msg = new PomMessage();
        msg->id = RailcomID::POM;
        msg->cvValue = buffer[1];
        return msg;
    }
    return nullptr;
}
