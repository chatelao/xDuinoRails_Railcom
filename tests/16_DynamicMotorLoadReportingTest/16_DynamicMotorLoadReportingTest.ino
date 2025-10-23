#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.4 - Message Type DYN
// A locomotive's decoder reports increased motor load as it begins to
// climb a grade, which can be used to trigger realistic sound effects.

test(MotorLoadReporting, reportLoad) {
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
