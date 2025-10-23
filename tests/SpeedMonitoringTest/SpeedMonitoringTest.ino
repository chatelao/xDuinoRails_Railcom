#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.4 - Message Type DYN
// The command station displays the actual, real-time speed of a locomotive
// as reported by its decoder.

test(SpeedMonitoring, monitorSpeed) {
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
