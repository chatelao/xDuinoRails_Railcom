#include "RailcomTx.h"
#include "RailcomRx.h"
#include "RailcomEncoding.h"
#include "mocks/MockRailcomHardware.h"

// Minimal testing framework
#define test(name) void test_##name()
#define run_test(name) Serial.print("Running test: "#name"... "); test_##name(); Serial.println("PASSED")
#define assertEqual(a, b) if (a != b) { Serial.print("FAILED: "); Serial.print((int)a); Serial.print(" != "); Serial.println((int)b); while(1); }
#define assertTrue(a) if (!(a)) { Serial.println("FAILED: assertion failed"); while(1); }
#define assertNotNull(a) if (a == nullptr) { Serial.println("FAILED: pointer is null"); while(1); }

// Verifies that the RailcomTx and RailcomRx classes can be instantiated.
test(creation) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  assertTrue(&tx != nullptr);
  assertTrue(&rx != nullptr);
}

// Verifies the end-to-end transmission and reception of a short address.
test(short_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t shortAddress = 100;

  // Send the low part of the address first (alternator is initially false).
  tx.sendAddress(shortAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, shortAddress & 0x7F);
  hardware.clear();

  // Send the high part of the address (alternator is now true).
  tx.sendAddress(shortAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 0);
  hardware.clear();
}

// Verifies that every message sent by RailcomTx can be correctly parsed by RailcomRx.
test(end_to_end) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // Verifies POM (ID 0) message sending and parsing.
  tx.sendPomResponse(42);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(msg)->cvValue, 42);
  hardware.clear();

  // Verifies ADR_HIGH (ID 1) message sending and parsing.
  tx.sendAddress(3);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 3);
  hardware.clear();

  // Verifies DYN (ID 7) message sending and parsing.
  tx.sendDynamicData(1, 100);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DYN);
  assertEqual(static_cast<DynMessage*>(msg)->subIndex, 1);
  assertEqual(static_cast<DynMessage*>(msg)->value, 100);
  hardware.clear();

  // Verifies XPOM (ID 8-11) message sending and parsing.
  uint8_t cvs[] = {1, 2, 3, 4};
  tx.sendXpomResponse(0, cvs);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::XPOM_0);
  XpomMessage* xpomMsg = static_cast<XpomMessage*>(msg);
  assertEqual(xpomMsg->sequence, 0);
  assertEqual(xpomMsg->cvValues[0], 1);
  assertEqual(xpomMsg->cvValues[1], 2);
  assertEqual(xpomMsg->cvValues[2], 3);
  assertEqual(xpomMsg->cvValues[3], 4);
  hardware.clear();

  // Verifies STAT1 (ID 4) message sending and parsing.
  tx.sendStatus1(0xAB);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT1);
  assertEqual(static_cast<Stat1Message*>(msg)->status, 0xAB);
  hardware.clear();

  // Verifies STAT2 (ID 8) message sending and parsing.
  tx.sendStatus2(0x11);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT2);
  assertEqual(static_cast<Stat2Message*>(msg)->status, 0x11);
  hardware.clear();

  // Verifies STAT4 (ID 3) message sending and parsing.
  tx.sendStatus4(0xCD);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(msg)->status, 0xCD);
  hardware.clear();

  // Verifies ERROR (ID 6) message sending and parsing.
  tx.sendError(0xEF);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ERROR);
  assertEqual(static_cast<ErrorMessage*>(msg)->errorCode, 0xEF);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 0.1s).
  tx.sendTime(127, false); // 12.7 seconds
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg1 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg1->unit_is_second, false);
  assertEqual(timeMsg1->timeValue, 127);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 1s).
  tx.sendTime(42, true); // 42 seconds
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg2 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg2->unit_is_second, true);
  assertEqual(timeMsg2->timeValue, 42);
  hardware.clear();

  // Verifies CV_AUTO (ID 12) message sending and parsing.
  tx.sendCvAuto(0x123456, 0xAB);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  CvAutoMessage* cvAutoMsg = static_cast<CvAutoMessage*>(msg);
  assertEqual(cvAutoMsg->cvAddress, 0x123456);
  assertEqual(cvAutoMsg->cvValue, 0xAB);
  hardware.clear();
}


void setup() {
  Serial.begin(115200);
  while (!Serial);

  run_test(creation);
  run_test(end_to_end);
  run_test(long_address_e2e);
  run_test(short_address_e2e);
  run_test(decoder_state_machine_e2e);
  run_test(srq_e2e);

  Serial.println("All tests passed!");
}

// Verifies the end-to-end transmission and reception of a Service Request.
test(srq_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t accessoryAddress = 1234;
  bool isExtended = true;

  tx.sendServiceRequest(accessoryAddress, isExtended);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();

  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::SRQ);
  SrqMessage* srqMsg = static_cast<SrqMessage*>(msg);
  assertEqual(srqMsg->accessoryAddress, accessoryAddress);
  assertEqual(srqMsg->isExtended, isExtended);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception of a long address.
test(long_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t longAddress = 4097; // Example long address (0x1001)

  // Send the high part of the address.
  // The internal alternator in RailcomTx starts with the high part.
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (longAddress >> 8) & 0x3F);
  hardware.clear();

  // Send the low part of the address.
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, longAddress & 0xFF);
  hardware.clear();
}

#include "DecoderStateMachine.h"

// Verifies the DecoderStateMachine's response to a POM read command.
test(decoder_state_machine_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 100);

  // Simulate a DCC POM read command for CV 1
  uint8_t dcc_data[] = {0, 100, 0b11100100, 1, 0}; // Address 100, Read CV 1
  DCCMessage msg(dcc_data, 5);
  sm.handleDccPacket(msg);

  // Verify that a POM response with the dummy value 42 is sent
  RailcomRx rx(&hardware);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(railcomMsg)->cvValue, 42);
}

void loop() {
  // Do nothing
}
