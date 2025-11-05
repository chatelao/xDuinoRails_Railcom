#include <Railcom.h>
#include <RailcomRx.h>
#include <RP2040RailcomHardware.h>

// Use the default UART instance and pins for the Railcom hardware
RP2040RailcomHardware railcomHardware(&uart0, /*TX pin*/ 0, /*RX pin*/ 1);
RailcomRx railcomRx(&railcomHardware);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial connection
  }
  Serial.println("Railcom Printer Example");

  railcomRx.begin();
}

void loop() {
  // Check if a complete Railcom message has been received
  if (railcomRx.read()) {
    Serial.println("--- New Railcom Message ---");
    railcomRx.print(Serial);
    Serial.println("--------------------------");
    Serial.println();
  }
}
