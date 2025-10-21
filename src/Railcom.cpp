#include "Railcom.h"
#include "railcom.pio.h"
#include <cstring>

// Include the generated PIO header
#if __has_include("railcom.pio.h")
#include "railcom.pio.h"
#else
// Fallback for when the header hasn't been generated yet
const struct pio_program railcom_pio_program = {
    .instructions = NULL,
    .length = 0,
    .origin = -1,
};
#endif

DCCMessage::DCCMessage() : _len(0) {
}

DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len) {
    memcpy(_data, data, len);
}

uint16_t DCCMessage::getAddress() const {
    if (_len < 2) return 0;
    return (_data[0] << 8) | _data[1];
}

uint8_t DCCMessage::getCommand() const {
    if (_len < 3) return 0;
    return _data[2];
}

const uint8_t* DCCMessage::getData() const {
    return _data;
}

size_t DCCMessage::getLength() const {
    return _len;
}

RailcomSender* RailcomSender::_instance = nullptr;
DCCMessage RailcomSender::_msg;

RailcomSender::RailcomSender(uart_inst_t* uart, uint txPin, uint rxPin)
    : _uart(uart), _txPin(txPin), _rxPin(rxPin), _pio(pio0), _sm(0) {
    _instance = this;
}

void RailcomSender::begin() {
    uart_init(_uart, 115200);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
    gpio_set_function(_rxPin, GPIO_FUNC_UART);
    pio_init();
}

void RailcomSender::end() {
    pio_sm_set_enabled(_pio, _sm, false);
    pio_remove_program(_pio, &railcom_pio_program, _sm);
    uart_deinit(_uart);
}

void RailcomSender::send_dcc_async(const DCCMessage& msg) {
    _msg = msg;
    // Disable UART to send DCC
    uart_deinit(_uart);
    // PIO to create the cutout
    pio_sm_put_blocking(_pio, _sm, 1772);
    pio_sm_set_enabled(_pio, _sm, true);
}

void RailcomSender::pio_init() {
    uint offset = pio_add_program(_pio, &railcom_cutout_program);
    pio_sm_config c = railcom_cutout_program_get_default_config(offset);
    sm_config_set_out_pins(&c, _txPin, 1);
    sm_config_set_set_pins(&c, _txPin, 1);
    sm_config_set_sideset_pins(&c, _txPin);
    pio_sm_init(_pio, _sm, offset, &c);
    pio_set_irq0_source_enabled(_pio, pis_interrupt0, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

void RailcomSender::pio_irq_handler() {
    if (pio_interrupt_get(_instance->_pio, 0)) {
        pio_interrupt_clear(_instance->_pio, 0);
        // Re-enable UART
        uart_init(_instance->_uart, 115200);
        gpio_set_function(_instance->_txPin, GPIO_FUNC_UART);
        uart_write_blocking(_instance->_uart, _msg.getData(), _msg.getLength());
        // Disable PIO
        pio_sm_set_enabled(_instance->_pio, _instance->_sm, false);
    }
}

RailcomReceiver::RailcomReceiver(uart_inst_t* uart, uint txPin, uint rxPin)
    : _uart(uart), _txPin(txPin), _rxPin(rxPin) {
}

void RailcomReceiver::begin() {
    uart_init(_uart, 115200);
    gpio_set_function(_txPin, GPIO_FUNC_UART);
    gpio_set_function(_rxPin, GPIO_FUNC_UART);
}

void RailcomReceiver::end() {
    uart_deinit(_uart);
}

DCCMessage RailcomReceiver::parse_dcc_message(const uint8_t* data, size_t len) {
    return DCCMessage(data, len);
}

void RailcomReceiver::set_decoder_address(uint16_t address) {
    _decoder_address = address;
}

void RailcomReceiver::handle_dcc_message(const DCCMessage& msg) {
    if (msg.getAddress() == 0xFFFF) { // Broadcast address
        if (msg.getCommand() == 0x01) { // Discovery request
            send_discovery_response();
        }
    } else if (msg.getAddress() == _decoder_address) {
        // Handle message for this decoder
    }
}

const std::vector<uint16_t>& RailcomReceiver::get_discovered_addresses() const {
    return _discovered_addresses;
}

void RailcomReceiver::send_discovery_response() {
    uint8_t data[] = {
        (uint8_t)(_decoder_address >> 8),
        (uint8_t)(_decoder_address & 0xFF)
    };
    uart_write_blocking(_uart, data, sizeof(data));
}

bool RailcomReceiver::read_response(uint8_t* buffer, size_t len, uint timeout_ms) {
    uint32_t start = millis();
    size_t received = 0;
    while(millis() - start < timeout_ms && received < len) {
        if (uart_is_readable(_uart)) {
            buffer[received++] = uart_getc(_uart);
        }
    }
    return received == len;
}
