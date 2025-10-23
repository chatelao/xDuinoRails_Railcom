#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.10 - Message Type SRQ
// An operator at a local panel presses a button; the accessory decoder
// sends a service request to the central station to activate a pre-set route.

test(RouteRequest, sendRequest) {
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
