#include <ArduinoUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.2 - Message Type ADR
// A RailCom detector reports not only that a block is occupied,
// but specifically *which* locomotive is in it.

const uint16_t LOCOMOTIVE_ADDRESS = 42;

test(BlockOccupancyReporting_sendsAdrOnSpeedCommand) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);

  DCCMessage dcc_msg = MockDcc::createSpeedPacket(LOCOMOTIVE_ADDRESS, 100);

  state_machine.handleDccPacket(dcc_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::ADR);
  assertEqual(railcom_tx.sentMessages[0].data1, LOCOMOTIVE_ADDRESS);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Test::run();
}

void loop() {
}
