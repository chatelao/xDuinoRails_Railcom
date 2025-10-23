#include <AUnit.h>
#include "Railcom.h"

// RCN-218, Chapter 4.2.4 - Data Space 4
// During logon, a RailComPlus decoder transmits which function (e.g., F1)
// corresponds to which icon (e.g., horn), automatically configuring
// the throttle display.

test(FunctionIconMapping, mapIcons) {
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
