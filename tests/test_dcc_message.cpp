#include <gtest/gtest.h>
#include "Railcom.h"

// Tests that the getAddress() function correctly extracts the address from a DCC message.
TEST(DCCMessageTest, GetAddress) {
    uint8_t data[] = {0x12, 0x34, 0x56};
    DCCMessage msg(data, sizeof(data));
    ASSERT_EQ(msg.getAddress(), 0x1234);
}

// Tests that the getCommand() function correctly extracts the command from a DCC message.
TEST(DCCMessageTest, GetCommand) {
    uint8_t data[] = {0x12, 0x34, 0x56};
    DCCMessage msg(data, sizeof(data));
    ASSERT_EQ(msg.getCommand(), 0x56);
}
