#include "Railcom.h"
#include "railcom.pio.h"
#include <cstring>

// ... (DCCMessage implementation is unchanged)

// --- RailcomSender Final Implementation ---
RailcomSender* pio_sender_instance = nullptr;

void railcom_pio_irq_handler() {
    if (pio_sender_instance && pio_interrupt_get(pio_sender_instance->_pio, 0)) {
        pio_interrupt_clear(pio_sender_instance->_pio, 0);
        pio_sender_instance->_send_pending = true;
        pio_sm_set_enabled(pio_sender_instance->_pio, pio_sender_instance->_sm, false);
    }
}

RailcomSender::RailcomSender(IRailcomStream* stream, uint pio_pin)
    : _stream(stream), _pio_pin(pio_pin), _pio(pio0), _sm(0), _offset(0), _send_pending(false) {
    pio_sender_instance = this;
}

void RailcomSender::begin() {
    _stream->begin(115200);
    pio_init();
}

void RailcomSender::end() {
    _stream->end();
    // ... (rest of end() is unchanged)
}

void RailcomSender::pio_init() {
    // ... (implementation is unchanged)
}

void RailcomSender::send_dcc_with_cutout(const DCCMessage& dccMsg) {
    _stream->write(dccMsg.getData(), dccMsg.getLength());
    _stream->tx_wait_blocking(); // Use robust wait

    gpio_set_function(_pio_pin, GPIO_FUNC_PIO0);
    pio_sm_put_blocking(_pio, _sm, 1772);
    pio_sm_set_enabled(_pio, _sm, true);
}

void RailcomSender::queue_message(uint8_t channel, const std::vector<uint8_t>& message) {
    // ... (implementation is unchanged)
}

void RailcomSender::task() {
    if (_send_pending) {
        _send_pending = false;

        _stream->setBaudrate(250000); // Use efficient baud rate change
        send_queued_messages();
        _stream->tx_wait_blocking(); // Robust wait
        _stream->setBaudrate(115200); // Switch back to DCC baud
    }
}

void RailcomSender::send_queued_messages() {
    // ... (implementation is unchanged)
}

// --- RailcomReceiver Implementation ---
// ... (implementation is unchanged)
