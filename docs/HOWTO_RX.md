# Howto: RailCom Detector (Receiver)

This guide explains how to wire a Seeed Xiao RP2040 and run a minimal software example to create a RailCom detector.

## Minimal Software Example

This example demonstrates how to initialize the `RailcomRx` library and read incoming RailCom messages.

**Code:**

```cpp
#include <Arduino.h>
#include "RailcomRx.h"
#include "RailcomDebug.h"

// Initialize the RailcomRx library on UART0, using D7 as the RX pin.
RailcomRx railcomRx(uart0, 1); // UART instance, RX Pin (GPIO1 -> D7)

void setup() {
  // Start the RailCom receiver
  railcomRx.begin();

  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect.
  }
  Serial.println("RailCom RX Example Initialized");
}

void loop() {
  // The railcomRx.task() function must be called repeatedly in the main loop
  // to process incoming data.
  railcomRx.task();

  // Check if a complete RailCom message has been received
  if (railcomRx.available()) {
    Railcom_message msg = railcomRx.read(); // Read the message from the buffer

    Serial.print("Received RailCom Message: ");
    printRailcomMessage(msg); // Use the debug helper to print a human-readable version
  }
}
```

**Explanation:**

1.  **`#include` directives:** We include the `RailcomRx.h` for the receiver logic and `RailcomDebug.h` for the handy `printRailcomMessage` function.
2.  **`RailcomRx railcomRx(...)`:** We create an instance of the `RailcomRx` class, telling it to use `uart0` and pin `1` (which is D7 on the Xiao RP2040) for receiving.
3.  **`setup()`:** We initialize the `RailcomRx` library.
4.  **`loop()`:**
    *   `railcomRx.task()`: This function checks the UART for incoming bytes and assembles them into messages. It must be called frequently.
    *   `railcomRx.available()`: This returns `true` when a complete, valid RailCom message has been received and is ready to be processed.
    *   `railcomRx.read()`: This retrieves the message from the library's internal buffer.
    *   `printRailcomMessage(msg)`: This is a helper function provided by the library that prints a formatted, human-readable description of the message to the Serial monitor, which is very useful for debugging.
