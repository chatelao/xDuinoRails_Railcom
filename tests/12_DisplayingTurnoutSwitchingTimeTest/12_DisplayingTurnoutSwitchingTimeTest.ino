#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.8 - Message Type TIME
// For a slow-motion turnout, the decoder reports the expected time until
// the movement is complete, allowing for prototypical signaling.

test(TurnoutSwitchingTime, displayTime) {
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
