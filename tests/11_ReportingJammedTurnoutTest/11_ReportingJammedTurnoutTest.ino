#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.9 - Message Type ERROR
// A turnout motor stalls or fails; the accessory decoder reports an error
// condition back to the control panel.

test(JammedTurnout, reportError) {
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
