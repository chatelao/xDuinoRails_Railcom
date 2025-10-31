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

// Verifies that every message sent by RailcomTx can be correctly parsed by RailcomRx.
test(end_to_end) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // Verifies POM (ID 0) message sending and parsing.
  tx.sendPomResponse(42);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(msg)->cvValue, 42);
  hardware.clear();

  // Verifies ADR_HIGH (ID 1) message sending and parsing.
  tx.sendAddress(3);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 3);
  hardware.clear();

  // Verifies DYN (ID 7) message sending and parsing.
  tx.sendDynamicData(1, 100);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DYN);
  assertEqual(static_cast<DynMessage*>(msg)->subIndex, 1);
  assertEqual(static_cast<DynMessage*>(msg)->value, 100);
  hardware.clear();

  // Verifies XPOM (ID 8-11) message sending and parsing.
  uint8_t cvs[] = {1, 2, 3, 4};
  tx.sendXpomResponse(0, cvs);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
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
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT1);
  assertEqual(static_cast<Stat1Message*>(msg)->status, 0xAB);
  hardware.clear();

  // Verifies STAT2 (ID 8) message sending and parsing.
  tx.sendStatus2(0x11);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT2);
  assertEqual(static_cast<Stat2Message*>(msg)->status, 0x11);
  hardware.clear();

  // Verifies STAT4 (ID 3) message sending and parsing.
  tx.sendStatus4(0xCD);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(msg)->status, 0xCD);
  hardware.clear();

  // Verifies ERROR (ID 6) message sending and parsing.
  tx.sendError(0xEF);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ERROR);
  assertEqual(static_cast<ErrorMessage*>(msg)->errorCode, 0xEF);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing.
  tx.sendTime(1, 42);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg->resolution, 1);
  assertEqual(timeMsg->time, 42);
  hardware.clear();

  // Verifies CV_AUTO (ID 12) message sending and parsing.
  tx.sendCvAuto(0x123456, 0xAB);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.readMessage();
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

  Serial.println("All tests passed!");
}

void loop() {
  // Do nothing
}
