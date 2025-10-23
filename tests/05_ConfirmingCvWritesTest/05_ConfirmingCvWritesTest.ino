#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.2 - Message Type POM
// After writing a new value to a CV on the main, the decoder
// sends back the new value to confirm it was set successfully.

const uint16_t LOCO_ADDRESS = 8765;
const uint16_t CV_TO_WRITE = 3;
const uint8_t CV_VALUE = 128;

test(ConfirmingCvWrites, sendsPomOnWritePacket) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

  DCCMessage dcc_msg = MockDcc::createPomWritePacket(LOCO_ADDRESS, CV_TO_WRITE, CV_VALUE);

  state_machine.handleDccPacket(dcc_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::POM);
  assertEqual(railcom_tx.sentMessages[0].data1, CV_VALUE);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
