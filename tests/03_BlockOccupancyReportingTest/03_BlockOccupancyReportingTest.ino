#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.1 - Message Type ADR
// A RailCom detector reports not only that a block is occupied,
// but specifically *which* locomotive is in it.

const uint16_t LOCO_ADDRESS = 2023;

test(BlockOccupancyReporting, sendsAdrOnPacket) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

  DCCMessage dcc_msg = MockDcc::createSpeedPacket(LOCO_ADDRESS, 10);

  state_machine.handleDccPacket(dcc_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::ADR_LOW);
  assertEqual(railcom_tx.sentMessages[0].data1, LOCO_ADDRESS);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
