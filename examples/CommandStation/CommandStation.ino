/**
 * @file CommandStation.ino
 * @brief This example demonstrates the use of the RailcomRx class to receive and process RailCom messages.
 * It shows how to poll for messages and use the built-in print function to display received data to the serial monitor.
 * This acts as a simple "RailCom Sniffer".
 *
 * @author Olivier Châtelain-Gmür
 */

#include <Arduino.h>
#include "RP2040RailcomRxHardware.h"
#include "RailcomRx.h"

// ====================================================================================================
// Global Objects
// ====================================================================================================

// Create an instance of the RP2040 hardware layer.
// The constructor takes the UART block (uart0 or uart1) and the GPIO pin number for the RailCom input.
RP2040RailcomRxHardware rxHardware(uart0, 0);

// Create an instance of the RailcomRx class, passing it the hardware layer.
RailcomRx rx(&rxHardware);

// ====================================================================================================
// Arduino Setup & Loop
// ====================================================================================================

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(1000); // Wait for Serial to be ready
  Serial.println("RailCom Command Station Example");
  Serial.println("Listening for RailCom messages...");

  // Initialize the RailCom receiver.
  rx.begin();

  // Set a default context. Some message IDs are ambiguous and depend on whether
  // the sender is a mobile (locomotive) or stationary (accessory) decoder.
  // We'll default to MOBILE here.
  rx.setContext(DecoderContext::MOBILE);
}

void loop() {
  // The main work is done by the read() method. It checks for incoming data,
  // decodes it, and parses it into a message structure.
  RailcomMessage* msg = rx.read();

  // If read() returns a non-null pointer, a valid message was received.
  if (msg != nullptr) {
    Serial.print("Received Message: ");
    // The print() method provides a human-readable output for the last message
    // received. It handles all message types and stateful information, like
    // combining ADR_HIGH and ADR_LOW into a full address.
    rx.print(Serial);
  }

  // A small delay to keep the loop from running too fast.
  delay(10);
}
