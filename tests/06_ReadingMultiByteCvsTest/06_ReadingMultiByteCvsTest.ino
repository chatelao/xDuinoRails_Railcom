#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.3 - Message Type XPOM
// Reading a CV that requires more than 8 bits of data (e.g., Manufacturer ID)
// using a sequence of extended POM messages.

const uint16_t LOCO_ADDRESS = 4321;
const uint16_t XPOM_CV_START = 100; // The CV that triggers the XPOM sequence
const uint32_t CV_VALUE = 0xAABBCCDD;

test(ReadingMultiByteCvs, sendsXpomSequence) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

  // First, write the four bytes of the 32-bit value to the appropriate CVs.
  state_machine.handleDccPacket(MockDcc::createPomWritePacket(LOCO_ADDRESS, XPOM_CV_START, (CV_VALUE >> 24) & 0xFF));
  state_machine.handleDccPacket(MockDcc::createPomWritePacket(LOCO_ADDRESS, XPOM_CV_START + 1, (CV_VALUE >> 16) & 0xFF));
  state_machine.handleDccPacket(MockDcc::createPomWritePacket(LOCO_ADDRESS, XPOM_CV_START + 2, (CV_VALUE >> 8) & 0xFF));
  state_machine.handleDccPacket(MockDcc::createPomWritePacket(LOCO_ADDRESS, XPOM_CV_START + 3, CV_VALUE & 0xFF));

  railcom_tx.clear();

  // Now, read the value back by triggering the XPOM CV.
  DCCMessage read_msg = MockDcc::createPomReadPacket(LOCO_ADDRESS, XPOM_CV_START);
  state_machine.handleDccPacket(read_msg);

  // We expect a total of 2 XPOM messages.
  assertEqual(railcom_tx.sentMessages.size(), 2);

  // Check the first XPOM message
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::XPOM_0);
  assertEqual(railcom_tx.sentMessages[0].data1, (CV_VALUE >> 16) & 0xFFFF);

  // Check the second XPOM message
  assertEqual(railcom_tx.sentMessages[1].id, RailcomID::XPOM_1);
  assertEqual(railcom_tx.sentMessages[1].data1, CV_VALUE & 0xFFFF);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
