#include <AUnit.h>
#include "Railcom.h"

// RCN-218, Chapter 4.2.5 - Data Space 5
// A RailComPlus command station reads and displays the locomotive's
// full name (e.g., "DB Class 218") as stored in the decoder.

test(LocoLongName, readName) {
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
