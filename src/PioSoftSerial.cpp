#include "PioSoftSerial.h"
#include "soft_serial_rx.pio.h"
#include "hardware/gpio.h"

PioSoftSerial::PioSoftSerial(PIO pio, uint rxPin)
    : _pio(pio), _rxPin(rxPin), _sm(0), _offset(0) {}

void PioSoftSerial::begin(uint32_t baudrate) {
    _sm = pio_claim_unused_sm(_pio, true);
    _offset = pio_add_program(_pio, &soft_serial_rx_program);

    pio_sm_config c = soft_serial_rx_program_get_default_config(_offset);

    // Configure the 'in' instruction to read from the specified pin.
    sm_config_set_in_pins(&c, _rxPin);
    // Configure the 'wait' instruction to use the same pin.
    sm_config_set_jmp_pin(&c, _rxPin);

    // Set the clock divider to get the correct baud rate.
    // The PIO program samples at 8x the baud rate.
    float div = (float)clock_get_hz(clk_sys) / (8 * baudrate);
    sm_config_set_clkdiv(&c, div);

    // Shift data from LSB first, and push when 8 bits are received.
    sm_config_set_in_shift(&c, false, true, 8);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);

    // Load the config and enable the state machine.
    pio_sm_init(_pio, _sm, _offset, &c);
    pio_sm_set_enabled(_pio, _sm, true);

    // Set the pin's function to PIO
    gpio_set_function(_rxPin, _pio == pio0 ? GPIO_FUNC_PIO0 : GPIO_FUNC_PIO1);
}

void PioSoftSerial::end() {
    pio_sm_set_enabled(_pio, _sm, false);
    pio_remove_program(_pio, &soft_serial_rx_program, _offset);
    pio_sm_unclaim(_pio, _sm);
}

int PioSoftSerial::available() {
    return pio_sm_get_rx_fifo_level(_pio, _sm);
}

int PioSoftSerial::read() {
    if (available() > 0) {
        // The PIO shifts in LSB first, so the bits are reversed.
        // We need to reverse them back before returning.
        uint32_t raw_byte = pio_sm_get(_pio, _sm);
        uint8_t reversed_byte = 0;
        for (int i = 0; i < 8; ++i) {
            reversed_byte |= ((raw_byte >> i) & 1) << (7 - i);
        }
        return reversed_byte;
    }
    return -1;
}

void PioSoftSerial::write(const uint8_t* data, size_t len) {
    // Not supported
}
