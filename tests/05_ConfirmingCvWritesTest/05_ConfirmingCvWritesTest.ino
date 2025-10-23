#include <AUnit.h>
#include "Railcom.h"

// RCN-217, Chapter 7.2.2 - Message Type POM
// After writing a new value to a CV on the main, the decoder
// sends back the new value to confirm it was set successfully.

test(PomWrite, confirmCv) {
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
