#include <AUnit.h>
#include "Railcom.h"

// RCN-218, Chapter 4.2.6 - Data Space 6
// A RailComPlus command station reads and displays product information,
// like the manufacturer and article number, from the decoder.

test(ManufacturerInfo, readInfo) {
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
