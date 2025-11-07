#include "RailcomDccParser.h"

void RailcomDccParser::parse(const DCCMessage& msg) {
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
    uint16_t address = (data[0] << 8) | data[1];

    if (len >= 3) {
        uint8_t byte3 = data[2];
        if ((byte3 & 0b11100000) == 0b11100000) { // POM
            uint16_t cv = (byte3 & 0x1F) << 8 | data[3];
            if ((byte3 & 0b00011100) == 0b00000100 && onPomReadCv) { // Read CV
                 onPomReadCv(cv, address);
            } else if ((byte3 & 0b00011100) == 0b00011100 && onPomWriteCv) { // Write CV
                 onPomWriteCv(cv, data[4], address);
            } else if ((byte3 & 0b00011100) == 0b00011000 && onPomWriteBit) { // Write Bit
                uint8_t bit = data[4] & 0x07;
                uint8_t value = (data[4] >> 3) & 1;
                onPomWriteBit(cv, bit, value, address);
            }
        }
    } else if ((data[0] & 0b11000000) == 0b10000000 && len >= 2 && onAccessory) { // Accessory
        address = 1 + (((~data[0]) & 0x3F) << 2) | ((data[1] >> 1) & 0x03);
        bool activate = (data[1] >> 3) & 1;
        uint8_t output = data[1] & 0x03;
        onAccessory(address, activate, output);
    } else if ((data[1] & 0b11010000) == 0b11010000 && onFunction) {
        uint8_t function = data[1] & 0x1F;
        bool state = (data[2] >> 5) & 1;
        onFunction(function, state);
    }
}
