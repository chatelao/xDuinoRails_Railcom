#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.5 - Message Type STAT1, STAT4
// An accessory decoder for a turnout reports back that it has successfully
// thrown and is now in the requested position.

test(TurnoutPosition, verifyPosition) {
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
