#include "RailcomDccParser.h"

void RailcomDccParser::parse(const DCCMessage& msg) {
    const uint8_t* data = msg.getData();
    size_t len = msg.getLength();

    if (len < 2 || data[0] != RCN218::DCC_A_ADDRESS) {
        return;
    }

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
}
