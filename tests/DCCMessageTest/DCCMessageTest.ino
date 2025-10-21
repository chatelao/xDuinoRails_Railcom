#include <AUnit.h>
#include <Railcom.h>

test(DCCMessage, getAddress) {
  uint8_t data[] = {0x12, 0x34, 0x56};
  DCCMessage msg(data, sizeof(data));
  assertEqual(msg.getAddress(), 0x1234);
}

test(DCCMessage, getCommand) {
  uint8_t data[] = {0x12, 0x34, 0x56};
  DCCMessage msg(data, sizeof(data));
  assertEqual(msg.getCommand(), 0x56);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB
  TestRunner::run();
}

void loop() {
}
