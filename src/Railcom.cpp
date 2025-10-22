#include "Railcom.h"
#include "railcom.pio.h"
#include <cstring>

// --- DCCMessage Implementation ---
DCCMessage::DCCMessage() : _len(0) {}
DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len > sizeof(_data) ? sizeof(_data) : len) {
    memcpy(_data, data, _len);
}
const uint8_t* DCCMessage::getData() const { return _data; }
size_t DCCMessage::getLength() const { return _len; }


// --- RailcomSender Implementation --- was here

// --- RailcomReceiver Implementation ---
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

// --- Railcom Encoding/Decoding Implementation ---

namespace Railcom {

// CRC8 lookup table (Polynomial 0x31, reversed 0x8C) for RCN-218 checksum calculation.
static const uint8_t crc_array[256] = {
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};

uint8_t crc8(const uint8_t* data, size_t len, uint8_t init) {
    uint8_t crc = init;
    for (size_t i = 0; i < len; ++i) {
        crc = crc_array[data[i] ^ crc];
    }
    return crc;
}

// Lookup table for 4-out-of-8 encoding (from RCN-217, Table 2).
// Stored in PROGMEM to save RAM.
static const uint8_t PROGMEM encoding_table[64] = {
    0b10101100, 0b10101010, 0b10101001, 0b10100101, 0b10100011, 0b10100110, 0b10011100, 0b10001110,
    0b10001101, 0b10010101, 0b10010011, 0b10010110, 0b10011001, 0b10011010, 0b10011100, 0b1001110,
    0b10110010, 0b10110100, 0b10111000, 0b01110100, 0b01110010, 0b01101100, 0b01101010, 0b01101001,
    0b01100101, 0b01100011, 0b01100110, 0b01011100, 0b01011010, 0b01011001, 0b01010101, 0b01010011,
    0b01010110, 0b01001110, 0b01001101, 0b01001011, 0b01000111, 0b01110001, 0b11101000, 0b11100100,
    0b11100010, 0b11010001, 0b11001001, 0b11000101, 0b11011000, 0b11010100, 0b11010010, 0b11001010,
    0b11000110, 0b11001100, 0b01111000, 0b00010111, 0b00011011, 0b00011101, 0b00011110, 0b00101110,
    0b00110110, 0b00111010, 0b00100111, 0b00101011, 0b00101101, 0b00110101, 0b00111001, 0b00110011
};

uint8_t encode4of8(uint8_t value) {
    if (value > 63) {
        return 0; // Invalid input, return a known-bad value.
    }
    return pgm_read_byte(&encoding_table[value]);
}

int16_t decode4of8(uint8_t encodedByte) {
    // Check for special ACK/NACK values first, as they don't follow the 4-of-8 rule.
    if (encodedByte == ACK1 || encodedByte == ACK2) {
        return 0x100; // Special code for ACK
    }
    if (encodedByte == NACK) {
        return 0x101; // Special code for NACK
    }

    // A valid 4-of-8 byte must have exactly four bits set to '1'.
    // This is a basic error check (Hamming weight).
    uint8_t weight = 0;
    for (int i = 0; i < 8; ++i) {
        if ((encodedByte >> i) & 1) {
            weight++;
        }
    }
    if (weight != 4) {
        return -1; // Invalid encoding
    }

    // Search the lookup table to find the corresponding 6-bit value.
    for (int i = 0; i < 64; ++i) {
        if (pgm_read_byte(&encoding_table[i]) == encodedByte) {
            return i;
        }
    }

    return -1; // Should be unreachable if Hamming weight is 4, but included for safety.
}

} // namespace Railcom
