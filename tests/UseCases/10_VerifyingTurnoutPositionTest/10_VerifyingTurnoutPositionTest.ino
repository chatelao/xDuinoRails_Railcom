#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.5 - Message Type STAT1
// An accessory decoder for a turnout reports back that it has successfully
// thrown and is now in the requested position.

const uint16_t ACCESSORY_ADDRESS = 101;
const bool THROWN = true;

test(VerifyingTurnoutPosition, sendsStat1OnAccessoryCommand) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::ACCESSORY, ACCESSORY_ADDRESS);

  DCCMessage dcc_msg = MockDcc::createAccessoryPacket(ACCESSORY_ADDRESS, THROWN);

  state_machine.handleDccPacket(dcc_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::STAT1);
  assertEqual(railcom_tx.sentMessages[0].data1, (uint32_t)(1 << 0)); // Turnout 1 is thrown
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
