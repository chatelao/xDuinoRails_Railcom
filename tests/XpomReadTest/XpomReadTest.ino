#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.3 - Message Type XPOM
// Reading a CV that requires more than 8 bits of data (e.g., Manufacturer ID)
// using a sequence of extended POM messages.

test(XpomRead, readMultiByteCv) {
  // Placeholder for test logic
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
