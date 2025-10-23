#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.1 - Message Type ADR
// A locomotive enters a specific block, and its unique address is used
// by control software to trigger a crossing signal and sound sequence.

test(AutomatedScenarios, triggerScenario) {
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
