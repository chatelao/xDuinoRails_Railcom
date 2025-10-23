#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.2 - Message Type POM
// A user requests to read the value of a CV from a specific
// locomotive directly on the main track.

test(PomRead, readCv) {
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
