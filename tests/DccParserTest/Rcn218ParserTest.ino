#include <AUnit.h>
#include "RailcomDccParser.h"

test(Rcn218Parser, LogonEnable) {
    RailcomDccParser parser;
    bool callback_fired = false;

    parser.onLogonEnable = [&](uint8_t group, uint16_t zid, uint8_t sessionId) {
        callback_fired = true;
        assertEqual(group, 1);
        assertEqual(zid, 0x1234);
        assertEqual(sessionId, 0x56);
    };

    uint8_t data[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ENABLE | 1, 0x12, 0x34, 0x56 };
    DCCMessage msg(data, sizeof(data));
    parser.parse(msg);

    assertTrue(callback_fired);
}

test(Rcn218Parser, Select) {
    RailcomDccParser parser;
    bool callback_fired = false;

    parser.onSelect = [&](uint16_t manufacturerId, uint32_t productId, uint8_t subCmd, const uint8_t* data, size_t len) {
        callback_fired = true;
        assertEqual(manufacturerId, 0x0123);
        assertEqual(productId, 0x456789AB);
        assertEqual(subCmd, 0xFC);
    };

    uint8_t data[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_SELECT | 1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xFC };
    DCCMessage msg(data, sizeof(data));
    parser.parse(msg);

    assertTrue(callback_fired);
}
