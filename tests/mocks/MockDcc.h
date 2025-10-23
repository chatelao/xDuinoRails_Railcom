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

} // namespace MockDcc

#endif // MOCK_DCC_H
