#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.11 - Message Type EXT
// A decoder equipped with a location sensor (e.g., GPS, Hall effect)
// reports its precise physical position on the layout.

test(LocationTracking, trackLocation) {
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
