#include "RailcomDccParser.h"

void RailcomDccParser::parse(const DCCMessage& msg, bool* response_sent) {
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();

    if (len < 2) return;

    // RCN-218 DCC-A Protocol
    if (data[0] == RCN218::DCC_A_ADDRESS) {
        uint8_t cmd = data[1];
        if (cmd >= RCN218::CMD_LOGON_ENABLE && cmd < 0xF4) {
            if (onLogonEnable && len >= 4) {
                uint8_t group = cmd & 0x03;
                uint16_t zid = (data[2] << 8) | data[3];
                uint8_t sessionId = data[4];
                onLogonEnable(group, zid, sessionId);
            }
        } else if (cmd >= RCN218::CMD_SELECT && cmd < 0xE0) {
            if (onSelect && len >= 8) {
                uint16_t manufacturerId = ((cmd & 0x0F) << 8) | data[2];
                uint32_t productId = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
                uint8_t subCmd = data[7];
                onSelect(manufacturerId, productId, subCmd, data + 8, len - 8);
            }
        } else if (cmd >= RCN218::CMD_LOGON_ASSIGN && cmd < 0xF0) {
            if (onLogonAssign && len >= 8) {
                uint16_t manufacturerId = ((cmd & 0x0F) << 8) | data[2];
                uint32_t productId = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
                uint16_t address = (data[7] << 8) | data[8];
                onLogonAssign(manufacturerId, productId, address);
            }
        } else {
            switch (cmd) {
                case RCN218::CMD_GET_DATA_START:
                    if (onGetDataStart) onGetDataStart();
                    break;
                case RCN218::CMD_GET_DATA_CONT:
                    if (onGetDataCont) onGetDataCont();
                    break;
                case RCN218::CMD_SET_DATA:
                    if (onSetData && len > 2) {
                        onSetData(data + 2, len - 2);
                    }
                    break;
                case RCN218::CMD_SET_DATA_END:
                    if (onSetDataEnd) onSetDataEnd();
                    break;
            }
        }
        return; // End of RCN-218 parsing
    }

    // RCN-217 Parsing
    // See RCN-217 Section 5.1 for POM command structure
    uint16_t address = (data[0] << 8) | data[1];

    if (len >= 3) {
        uint8_t byte3 = data[2];
        // Check for POM command: 111xxxxx
        if ((byte3 & 0b11100000) == 0b11100000) {
            if (response_sent) *response_sent = true;
            uint16_t cv = (byte3 & 0x1F) << 8 | data[3];
            // Check for Read CV command: 111001xx
            if ((byte3 & 0b00011100) == 0b00000100 && onPomReadCv) {
                 onPomReadCv(cv, address);
            // Check for Write CV command: 111111xx
            } else if ((byte3 & 0b00011100) == 0b00011100 && onPomWriteCv) {
                 onPomWriteCv(cv, data[4], address);
            // Check for Write Bit command: 111110xx
            } else if ((byte3 & 0b00011100) == 0b00011000 && onPomWriteBit) {
                uint8_t bit = data[4] & 0x07;
                uint8_t value = (data[4] >> 3) & 1;
                onPomWriteBit(cv, bit, value, address);
            }
        }
    // See RCN-217 Section 6 for accessory decoder commands
    } else if ((data[0] & 0b11000000) == 0b10000000 && len >= 2 && onAccessory) { // Accessory
        if (response_sent) *response_sent = true;
        address = 1 + (((~data[0]) & 0x3F) << 2) | ((data[1] >> 1) & 0x03);
        bool activate = (data[1] >> 3) & 1;
        uint8_t output = data[1] & 0x03;
        onAccessory(address, activate, output);
    // See RCN-212 Section 2.3.5 for binary state control commands
    } else if ((data[1] & 0b11010000) == 0b11010000 && onFunction) {
        if (response_sent) *response_sent = true;
        uint8_t function = data[1] & 0x1F;
        bool state = (data[2] >> 5) & 1;
        onFunction(address, function, state);
    // See RCN-212 Section 2.3.6 for extended function commands
    } else if (onExtendedFunction) {
        bool handled = false;
        uint16_t address = 0;
        uint8_t command = 0;

        // Long address format: ADDR_H, ADDR_L, 0xDE, CMD
        if (len == 4 && (data[0] & 0xC0) == 0xC0 && data[2] == 0xDE) {
            address = ((data[0] & 0x3F) << 8) | data[1];
            command = data[3];
            handled = true;
        // Short address format: ADDR, 0xDE, CMD
        } else if (len == 3 && (data[0] & 0x80) == 0 && data[1] == 0xDE) {
            address = data[0];
            command = data[2];
            handled = true;
        }

        if (handled) {
            if (response_sent) *response_sent = true;
            onExtendedFunction(address, command);
        }
    }
}
