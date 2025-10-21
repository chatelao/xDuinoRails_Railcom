#include "RailcomSender.h"
#include "railcom.pio.h"

RailcomSender* pio_sender_instance = nullptr;

void railcom_pio_irq_handler() {
    if (pio_sender_instance && pio_interrupt_get(pio_sender_instance->_pio, 0)) {
        pio_interrupt_clear(pio_sender_instance->_pio, 0);
        pio_sender_instance->_send_pending = true;
        pio_sm_set_enabled(pio_sender_instance->_pio, pio_sender_instance->_sm, false);
    }
}

RailcomSender::RailcomSender(uart_inst_t* uart, uint tx_pin, uint pio_pin)
    : _uart(uart), _tx_pin(tx_pin), _pio_pin(pio_pin), _pio(pio0), _sm(0), _offset(0), _send_pending(false) {
    pio_sender_instance = this;
}

void RailcomSender::begin() {
    uart_init(_uart, 115200);
    gpio_set_function(_tx_pin, GPIO_FUNC_UART);
    pio_init();
}

void RailcomSender::end() {
    uart_deinit(_uart);
    pio_sm_set_enabled(_pio, _sm, false);
    if (pio_can_remove_program(_pio, &railcom_cutout_program)) {
        pio_remove_program(_pio, &railcom_cutout_program, _offset);
    }
    pio_sm_unclaim(_pio, _sm);
}

void RailcomSender::pio_init() {
    _sm = pio_claim_unused_sm(_pio, true);
    _offset = pio_add_program(_pio, &railcom_cutout_program);
    pio_sm_config c = railcom_cutout_program_get_default_config(_offset);
    sm_config_set_out_pins(&c, _pio_pin, 1);
    sm_config_set_set_pins(&c, _pio_pin, 1);
    pio_sm_init(_pio, _sm, _offset, &c);
    pio_set_irq0_source_enabled(_pio, pis_interrupt0, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, railcom_pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

void RailcomSender::begin_cutout_sequence(const DCCMessage& dccMsg) {
    uart_write_blocking(_uart, dccMsg.getData(), dccMsg.getLength());
    uart_tx_wait_blocking(_uart);

    gpio_set_function(_pio_pin, GPIO_FUNC_PIO0);
    pio_sm_put_blocking(_pio, _sm, 1772);
    pio_sm_set_enabled(_pio, _sm, true);
}

void RailcomSender::queue_message(uint8_t channel, const std::vector<uint8_t>& message) {
    if (channel == 1) _ch1_queue.push(message);
    else _ch2_queue.push(message);
}

void RailcomSender::task() {
    if (_send_pending) {
        _send_pending = false;
        uart_set_baudrate(_uart, 250000);
        gpio_set_function(_tx_pin, GPIO_FUNC_UART);
        send_queued_messages();
        uart_tx_wait_blocking(_uart);
        uart_set_baudrate(_uart, 115200);
    }
}

void RailcomSender::send_queued_messages() {
    if (!_ch1_queue.empty()) {
        const auto& msg = _ch1_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch1_queue.pop();
    }
    sleep_us(193);
    while (!_ch2_queue.empty()) {
        const auto& msg = _ch2_queue.front();
        uart_write_blocking(_uart, msg.data(), msg.size());
        _ch2_queue.pop();
    }
}
