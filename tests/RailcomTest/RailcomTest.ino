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

test(creation) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  assertTrue(&tx != nullptr);
  assertTrue(&rx != nullptr);
}

test(sendStatus2) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  tx.sendStatus2(0x11);

  std::vector<uint8_t> expected = {0x72, 0x4D};
  assertEqual(hardware.getQueuedMessages().size(), 2);
  assertTrue(hardware.getQueuedMessages() == expected);
}

test(sendTime) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  tx.sendTime(1, 42); // resolution 1s, time 42s

  // ID=5 (0101), payload=0xAA (10101010)
  // chunks: 010110 (22) -> 0x59
  //         101010 (42) -> 0x99
  std::vector<uint8_t> expected = {0x59, 0x99};
  assertEqual(hardware.getQueuedMessages().size(), 2);
  assertTrue(hardware.getQueuedMessages() == expected);
}

test(sendCvAuto) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  tx.sendCvAuto(0x123456, 0xAB);

  // ID=12 (1100), payload=0x123456AB
  // chunks: 110000 (48) -> 0xA9
  //         010010 (18) -> 0x4E
  //         001101 (13) -> 0x3A
  //         000101 (5)  -> 0x27
  //         011010 (26) -> 0x65
  //         101011 (43) -> 0x9A
  std::vector<uint8_t> expected = {0xA9, 0x4E, 0x3A, 0x27, 0x65, 0x9A};
  assertEqual(hardware.getQueuedMessages().size(), 6);
  assertTrue(hardware.getQueuedMessages() == expected);
}

test(decode4of8) {
  assertEqual(RailcomEncoding::decode4of8(0x0F), 0);
  assertEqual(RailcomEncoding::decode4of8(0x99), 42);
  assertEqual(RailcomEncoding::decode4of8(0xFF), -1); // Invalid
}

test(parseMessage) {
  MockRailcomHardware hardware;
  RailcomRx rx(&hardware);
  std::vector<uint8_t> encodedBytes = {0x59, 0x99}; // TIME message, resolution 1, time 42
  hardware.setRxBuffer(encodedBytes);

  RailcomMessage* msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg->resolution, 1);
  assertEqual(timeMsg->time, 42);
}

test(parseAllMessages) {
  MockRailcomHardware hardware;
  RailcomRx rx(&hardware);

  // POM
  hardware.setRxBuffer({0x0F, 0x99}); // ID 0, value 42
  RailcomMessage* msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(msg)->cvValue, 42);

  // ADR_HIGH
  hardware.setRxBuffer({0x1E, 0x2B}); // ID 1, address 3
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 3);

  // STAT2
  hardware.setRxBuffer({0x72, 0x4D}); // ID 8, status 0x11
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT2);
  assertEqual(static_cast<Stat2Message*>(msg)->status, 0x11);

  // CV_AUTO
  hardware.setRxBuffer({0xA9, 0x4E, 0x3A, 0x27, 0x65, 0x9A}); // ID 12, addr 0x123456, value 0xAB
  msg = rx.readMessage();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvAddress, 0x123456);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvValue, 0xAB);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  run_test(creation);
  run_test(sendStatus2);
  run_test(sendTime);
  run_test(sendCvAuto);
  run_test(decode4of8);
  run_test(parseMessage);
  run_test(parseAllMessages);

  Serial.println("All tests passed!");
}

void loop() {
  // Do nothing
}
