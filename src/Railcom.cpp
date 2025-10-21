#include "Railcom.h"
#include "railcom.pio.h"
#include <cstring>

// --- DCCMessage Implementation ---
DCCMessage::DCCMessage() : _len(0) {}

DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len) {
    if (len > sizeof(_data)) len = sizeof(_data);
    memcpy(_data, data, len);
}

const uint8_t* DCCMessage::getData() const { return _data; }
size_t DCCMessage::getLength() const { return _len; }

// --- RailcomSender Implementation ---
RailcomSender* pio_sender_instance = nullptr;

void railcom_pio_irq_handler() {
    if (pio_sender_instance && pio_interrupt_get(pio_sender_instance->_pio, 0)) {
        pio_interrupt_clear(pio_sender_instance->_pio, 0);

        // Re-enable UART at RailCom baud rate. This is time-critical.
        uart_init(pio_sender_instance->_uart, 250000);
        gpio_set_function(pio_sender_instance->_txPin, GPIO_FUNC_UART);

        // Set the flag for the main loop to handle the rest.
        pio_sender_instance->_send_pending = true;

        pio_sm_set_enabled(pio_sender_instance->_pio, pio_sender_instance->_sm, false);
    }
}

RailcomSender::RailcomSender(uart_inst_t* uart, uint txPin, uint rxPin)
    : _uart(uart), _txPin(txPin), _rxPin(rxPin), _pio(pio0), _sm(0), _send_pending(false) {
    pio_sender_instance = this;
}

void RailcomSender::begin() {
    uart_init(_uart, 115200);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
    gpio_set_function(_rxPin, GPIO_FUNC_UART);
    pio_init();
}

void RailcomSender::end() {
    // Implementation remains the same
}

void RailcomSender::pio_init() {
    // Implementation remains the same
}

void RailcomSender::send_dcc_with_cutout(const DCCMessage& dccMsg) {
    // 1. Send the DCC message
    uart_write_blocking(_uart, dccMsg.getData(), dccMsg.getLength());
    uart_tx_wait_blocking(_uart);

    // 2. De-init UART and start PIO for the cutout
    uart_deinit(_uart);
    gpio_set_function(_txPin, GPIO_FUNC_PIO0);

    pio_sm_put_blocking(_pio, _sm, 1772);
    pio_sm_set_enabled(_pio, _sm, true);
}

void RailcomSender::queue_message(uint8_t channel, const std::vector<uint8_t>& message) {
    if (channel == 1) {
        _ch1_queue.push(message);
    } else if (channel == 2) {
        _ch2_queue.push(message);
    }
}

void RailcomSender::task() {
    if (_send_pending) {
        _send_pending = false;
        send_queued_messages();
    }
}

void RailcomSender::send_queued_messages() {
    if (!_ch1_queue.empty()) {
        const auto& msg = _ch1_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch1_queue.pop();
    }

    sleep_us(193); // Delay between channels

    while (!_ch2_queue.empty()) {
        const auto& msg = _ch2_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch2_queue.pop();
    }

    // Restore UART for next DCC packet
    uart_tx_wait_blocking(_uart);
    uart_init(_uart, 115200);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
}


// --- RailcomReceiver Implementation ---
// ... (implementation is unchanged for now)
RailcomReceiver::RailcomReceiver(uart_inst_t* uart, uint txPin, uint rxPin)
    : _uart(uart), _txPin(txPin), _rxPin(rxPin), _decoder_address(0) {}

void RailcomReceiver::begin() {
    uart_init(_uart, 250000);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
    gpio_set_function(_rxPin, GPIO_FUNC_UART);
}

void RailcomReceiver::end() {
    uart_deinit(_uart);
}

void RailcomReceiver::set_decoder_address(uint16_t address) {
    _decoder_address = address;
}

bool RailcomReceiver::read_response(std::vector<uint8_t>& buffer, uint timeout_ms) {
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
