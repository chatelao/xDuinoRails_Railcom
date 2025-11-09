#ifndef MOCK_DCC_H
#define MOCK_DCC_H

#include "Railcom.h"

// Provides helper functions to create mock DCCMessage objects for testing.
namespace MockDcc {

/**
 * @brief Creates a DCC accessory decoder packet.
 */
DCCMessage createAccessoryPacket(uint16_t address, bool thrown) {
    uint8_t addr_high = 0b10000000 | ((address - 1) >> 2);
    uint8_t addr_low = ((address - 1) & 0b11) << 4;
    uint8_t cmd_byte = 0b10001000 | addr_low | (thrown ? 1 : 0);
    uint8_t data[] = {addr_high, cmd_byte, (uint8_t)(addr_high ^ cmd_byte)};
    return DCCMessage(data, sizeof(data));
}

/**
 * @brief Creates a DCC speed and direction packet for a locomotive.
 */
DCCMessage createSpeedPacket(uint16_t address, uint8_t speed) {
    if (address > 127) { // Long address
        uint8_t addr_high = 0b11000000 | (address >> 8);
        uint8_t addr_low = address & 0xFF;
        uint8_t cmd_byte = 0b01100000 | (speed & 0x1F); // Simple speed for now
        uint8_t data[] = {addr_high, addr_low, cmd_byte, (uint8_t)(addr_high ^ addr_low ^ cmd_byte)};
        return DCCMessage(data, sizeof(data));
    } else { // Short address
        uint8_t addr_byte = address;
        uint8_t cmd_byte = 0b01100000 | (speed & 0x1F);
        uint8_t data[] = {addr_byte, cmd_byte, (uint8_t)(addr_byte ^ cmd_byte)};
        return DCCMessage(data, sizeof(data));
    }
}

/**
 * @brief Creates a generic DCC-A (RailComPlus) broadcast packet.
 */
DCCMessage createDccAPacket(uint8_t command) {
    uint8_t dccA_address = RCN218::DCC_A_ADDRESS;
    uint8_t data[] = {dccA_address, command, (uint8_t)(dccA_address ^ command)};
    return DCCMessage(data, sizeof(data));
}

/**
 * @brief Creates a DCC-A Logon Assign packet.
 */
DCCMessage createDccALogonAssignPacket(uint8_t handle, uint16_t address) {
    uint8_t dccA_address = RCN218::DCC_A_ADDRESS;
    uint8_t command = RCN218::CMD_LOGON_ASSIGN | handle;
    uint8_t addr_high = address >> 8;
    uint8_t addr_low = address & 0xFF;
    uint8_t checksum = dccA_address ^ command ^ addr_high ^ addr_low;
    uint8_t data[] = {dccA_address, command, addr_high, addr_low, checksum};
    return DCCMessage(data, sizeof(data));
}

// Creates a DCC message for an accessory decoder.
// See NMRA S-9.2.1
DCCMessage createAccessoryDccMessage(uint16_t address, bool activate, uint8_t output) {
    // Accessory addresses are mapped to a specific byte format.
    // Address bits 1-6 are in the first byte.
    // Address bits 7-9 are inverted and in the second byte.
    uint8_t byte1 = 0b10000000 | ((address - 1) & 0b00111111);
    uint8_t byte2 = 0b10001000 | (~(address >> 6) & 0b00000111) | (activate ? 0b00000001 : 0) | (output & 0b00000110);

    uint8_t dcc_data[] = {byte1, byte2, (uint8_t)(byte1 ^ byte2)};
    return DCCMessage(dcc_data, 3);
}

} // namespace MockDcc

#endif // MOCK_DCC_H
