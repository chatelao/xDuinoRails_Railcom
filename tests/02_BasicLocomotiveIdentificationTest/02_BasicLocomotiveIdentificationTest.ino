#include <AUnit.h>
#include "Railcom.h"
#include "RailcomRx.h"

// RCN-217, Chapter 7.2.1 - Message Type ADR
// A locomotive is placed on the track and its address is broadcast,
// allowing a detector to identify it.

test(LocoIdentification, broadcast) {
  // For the RP2040, UART0 is typically used for Serial communication.
  // We'll use UART1 for RailCom to avoid conflicts.
  RailcomRx railcom_rx(uart1, 1); // RX Pin 1 (GPIO1)

  // This is a basic test to ensure the RailcomRx object can be created.
  // The actual test logic will be added in a subsequent step.
  assertTrue(true);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
