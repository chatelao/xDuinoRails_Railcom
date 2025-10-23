#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.5 - Message Type STAT2
// A decoder for a semaphore signal or other mechanical accessory reports
// the precise physical position of its moving parts.

test(MechanicalDriveStatus, reportStatus) {
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
