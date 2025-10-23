#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.1 - Message Type ADR
// A locomotive placed on the track after derailing immediately broadcasts
// its address to signal it is powered and ready.

test(RerailingDetection, detectRerailing) {
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
