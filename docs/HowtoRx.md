# Howto: RailCom Detector (Receiver)

This guide explains how to wire a Seeed Xiao RP2040 and run a minimal software example to create a RailCom detector.

## Wiring

The following wiring is required to connect the Seeed Xiao RP2040 to your DCC track to receive RailCom messages.

**Components:**
- 1x Seeed Xiao RP2040
- 1x 6N137 Optocoupler
- Resistors: 1x 220Ω, 1x 1kΩ, 1x 10kΩ
- 1x 1N4148 diode

**Connections:**
This circuit uses an optocoupler to safely receive the RailCom signal from the high-voltage DCC track.

1.  **DCC Input Side:**
    *   Connect the DCC signal from your command station to one input of your track section.
    *   Connect the other DCC line to the other input of your track section.
    *   Connect the DCC signal line to the anode of the 1N4148 diode.
    *   Connect the cathode of the diode to pin 2 of the 6N137 optocoupler through a 220Ω resistor.
    *   Connect the other DCC line to pin 3 of the 6N137.

2.  **RP2040 Output Side:**
    *   Connect the 3.3V pin of the Xiao RP2040 to pin 8 of the 6N137.
    *   Connect the GND pin of the Xiao RP2040 to pin 5 of the 6N137.
    *   Connect a 10kΩ pull-up resistor between pin 6 of the 6N137 and the 3.3V pin of the RP2040.
    *   Connect pin 6 of the 6N137 to the RX pin of the Seeed Xiao RP2040's UART0 (Pin D7/GPIO1).

**Diagram:**

A full schematic for a RailCom detector can be quite complex. A simplified representation is below:

```
           ┌──────────┐                                  ┌──────────────┐
DCC Signal─┤ Diode+Resistor ├─(Pin 2) │ 6N137 Opto │ (Pin 6)──┬──(RX) D7/GPIO1 on Xiao RP2040
           └──────────┘              │          │          │
                                     └──────────┘          │
                                                         10kΩ Resistor
                                                           │
                                                           └---- 3.3V on Xiao RP2040
```
*Note: A complete and robust detector circuit is recommended for reliable operation. You can find many examples online by searching for "RailCom detector circuit".*

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
