#ifndef MOCK_DCC_H
#define MOCK_DCC_H

#include "Railcom.h"

// Provides helper functions to create mock DCCMessage objects for testing.
namespace MockDcc {

/**
 * @brief Creates a standard DCC speed/direction packet for short or long addresses.
 */
DCCMessage createSpeedPacket(uint16_t address, uint8_t speed, bool forward = true) {
    uint8_t speed_byte = 0b01000000;
    speed_byte |= (forward ? 0b00100000 : 0);

    uint8_t speed_val = speed & 0x0F;
    if (speed_val > 1) {
        speed_val |= 0b00010000;
    }
    speed_byte |= speed_val;

    if (address <= 127) {
        uint8_t addr_byte = address;
        uint8_t data[] = {addr_byte, speed_byte, (uint8_t)(addr_byte ^ speed_byte)};
        return DCCMessage(data, sizeof(data));
    } else {
        uint8_t addr_high = 0b11000000 | (address >> 8);
        uint8_t addr_low = address & 0xFF;
        uint8_t data[] = {addr_high, addr_low, speed_byte, (uint8_t)(addr_high ^ addr_low ^ speed_byte)};
        return DCCMessage(data, sizeof(data));
    }
}

/**
 * @brief Creates a DCC function group packet (F0-F4).
 */
DCCMessage createFunctionPacketF0toF4(uint16_t address, bool f0, bool f1, bool f2, bool f3, bool f4) {
    uint8_t cmd_byte = 0b10000000;
    if (f0) cmd_byte |= 0b00010000;
    if (f1) cmd_byte |= 0b00000001;
    if (f2) cmd_byte |= 0b00000010;
    if (f3) cmd_byte |= 0b00000100;
    if (f4) cmd_byte |= 0b00001000;

    if (address <= 127) {
        uint8_t addr_byte = address;
        uint8_t data[] = {addr_byte, cmd_byte, (uint8_t)(addr_byte ^ cmd_byte)};
        return DCCMessage(data, sizeof(data));
    } else {
        uint8_t addr_high = 0b11000000 | (address >> 8);
        uint8_t addr_low = address & 0xFF;
        uint8_t data[] = {addr_high, addr_low, cmd_byte, (uint8_t)(addr_high ^ addr_low ^ cmd_byte)};
        return DCCMessage(data, sizeof(data));
    }
}


/**
 * @brief Creates a DCC "Program on Main" (POM) write packet for a long address.
 */
DCCMessage createPomWritePacket(uint16_t address, uint16_t cv, uint8_t value) {
    uint8_t addr_high = 0b11000000 | (address >> 8);
    uint8_t addr_low = address & 0xFF;
    uint8_t cmd_byte = 0b11101100; // CV Write Instruction
    uint8_t cv_byte = cv - 1;
    uint8_t value_byte = value;
    uint8_t xor_byte = addr_high ^ addr_low ^ cmd_byte ^ cv_byte ^ value_byte;

    uint8_t data[] = {addr_high, addr_low, cmd_byte, cv_byte, value_byte, xor_byte};
    return DCCMessage(data, sizeof(data));
}

/**
 * @brief Creates a DCC "Program on Main" (POM) read packet for a long address.
 */
DCCMessage createPomReadPacket(uint16_t address, uint16_t cv) {
    uint8_t addr_high = 0b11000000 | (address >> 8);
    uint8_t addr_low = address & 0xFF;
    uint8_t cmd_byte = 0b11100100; // CV Read Instruction
    uint8_t cv_byte = cv - 1;
    uint8_t xor_byte = addr_high ^ addr_low ^ cmd_byte ^ cv_byte;

    uint8_t data[] = {addr_high, addr_low, cmd_byte, cv_byte, xor_byte};
    return DCCMessage(data, sizeof(data));
}

} // namespace MockDcc

#endif // MOCK_DCC_H
