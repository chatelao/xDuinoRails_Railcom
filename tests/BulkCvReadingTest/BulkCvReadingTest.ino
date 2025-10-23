#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.7 - Message Type CV-Car
// A technician triggers a function to have a passenger car's decoder
// transmit all its CV values for diagnostics.

test(BulkCvReading, readAllCvs) {
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
