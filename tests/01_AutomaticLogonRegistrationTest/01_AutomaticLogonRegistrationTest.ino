#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-218, Chapter 3 - Logon Procedure
// A RailComPlus-enabled locomotive automatically registers with the command station
// using its unique ID.

const uint16_t LOCO_ADDRESS = 100;
const uint32_t DECODER_ID = 0x12345678;

test(AutomaticLogonRegistration, respondsWithUniqueId) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS, 0, DECODER_ID);

  uint8_t data[] = {RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ENABLE, 0};
  data[2] = data[0] ^ data[1];
  DCCMessage dcc_msg(data, sizeof(data));

  state_machine.handleDccPacket(dcc_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::DECODER_UNIQUE);
  assertEqual(railcom_tx.sentMessages[0].data2, DECODER_ID);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
