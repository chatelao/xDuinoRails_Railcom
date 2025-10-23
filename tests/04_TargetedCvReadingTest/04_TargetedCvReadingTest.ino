#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.2 - Message Type POM
// A user requests to read the value of a CV from a specific
// locomotive directly on the main track.

const uint16_t LOCO_ADDRESS = 5678;
const uint16_t CV_TO_READ = 29;
const uint8_t CV_VALUE = 42;

test(TargetedCvReading, sendsPomOnReadPacket) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

  // We need a way to set the CV value in the decoder to test the response.
  // Since the real state machine doesn't have test hooks, we'll
  // first write the value using a POM write, then read it back.
  DCCMessage write_msg = MockDcc::createPomWritePacket(LOCO_ADDRESS, CV_TO_READ, CV_VALUE);
  state_machine.handleDccPacket(write_msg);

  // Now, read the value back.
  DCCMessage read_msg = MockDcc::createPomReadPacket(LOCO_ADDRESS, CV_TO_READ);
  state_machine.handleDccPacket(read_msg);

  // We expect two messages in the queue now: one for the write, one for the read.
  assertEqual(railcom_tx.sentMessages.size(), 2);
  assertEqual(railcom_tx.sentMessages[1].id, RailcomID::POM);
  assertEqual(railcom_tx.sentMessages[1].data1, CV_VALUE);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
