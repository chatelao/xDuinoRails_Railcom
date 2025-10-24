#ifndef RAILCOM_PROTOCOL_DEFS_H
#define RAILCOM_PROTOCOL_DEFS_H

// --- PIO Configuration ---
constexpr uint32_t PIO_CUTOUT_PULSE_WIDTH = 1772; // As per RCN-217

// --- UART Configuration ---
constexpr uint32_t UART_DCC_BAUDRATE = 115200;
constexpr uint32_t UART_RAILCOM_BAUDRATE = 250000;

// --- Timing ---
constexpr uint32_t RAILCOM_CH2_DELAY_US = 193; // Delay between CH1 and CH2

// --- Address Ranges ---
constexpr uint16_t MIN_SHORT_ADDRESS = 1;
constexpr uint16_t MAX_SHORT_ADDRESS = 127;
constexpr uint16_t MAX_ACCESSORY_ADDRESS = 2047;

// --- XPOM ---
constexpr uint8_t MAX_XPOM_SEQUENCE = 3;

// --- Rerailing ---
constexpr uint8_t MAX_RERAIL_SECONDS = 255;

// --- Data Space ---
constexpr uint8_t DATASPACE_LEN_MASK = 0x1F;

#endif // RAILCOM_PROTOCOL_DEFS_H
