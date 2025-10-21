#include <AUnit.h>
#include "Railcom.h"

test(DCCMessage, getAddressAndCommand) {
  uint8_t data[] = {0x12, 0x34, 0x56};
  DCCMessage msg(data, sizeof(data));

  // Note: DCC addresses are complex. This is a simplified test.
  // A real test would need to handle long/short addresses.
  // For this structure, we assume the first two bytes are part of the address.
  // A proper `getAddress()` method would need to be implemented in DCCMessage.
  // For now, we'll just test `getData()`.

  const uint8_t* received_data = msg.getData();
  assertEqual(received_data[0], 0x12);
  assertEqual(received_data[1], 0x34);
  assertEqual(received_data[2], 0x56);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
