#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.6 - Message Type INFO1
// A locomotive is programmed to join a consist and reports its orientation
// (forward/reverse) to the command station.

test(ConsistConfiguration, configureConsist) {
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
