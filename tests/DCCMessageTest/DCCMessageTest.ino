#include <ArduinoUnit.h>
#include "Railcom.h"

test(DCCMessage_getAddressAndCommand) {
  uint8_t data[] = {0x12, 0x34, 0x56};
  DCCMessage msg(data, sizeof(data));

  const uint8_t* received_data = msg.getData();
  assertEqual(received_data[0], 0x12);
  assertEqual(received_data[1], 0x34);
  assertEqual(received_data[2], 0x56);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
}

void loop() {
  Test::run();
}
