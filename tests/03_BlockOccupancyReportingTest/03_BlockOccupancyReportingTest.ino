#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.1 - Message Type ADR
// A RailCom detector reports not only that a block is occupied,
// but specifically *which* locomotive is in it.

test(BlockOccupancy, report) {
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
