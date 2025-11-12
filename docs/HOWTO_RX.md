# Howto: RailCom Detector (Receiver)

This guide explains how to wire a Seeed Studio XIAO RP2040 and run a minimal software example to create a RailCom detector, which could be part of a command station or a standalone track occupancy detector.

## Key Concepts

A RailCom receiver's job is to listen for messages sent by decoders during the DCC "cutout" period.

1.  **The Cutout:** This is the brief, powerless gap in the DCC signal that a command station creates. Decoders transmit their RailCom messages during this window.
2.  **Detector Circuit:** You cannot connect the XIAO RP2040 directly to the DCC track. The high-voltage DCC signal would damage it. You need a **RailCom detector circuit** that sits between the track and the microcontroller. This circuit's job is to isolate the microcontroller from the high track voltage and convert the faint RailCom current signal from the track into a clean logic-level (0-3.3V) serial signal that the XIAO's UART RX pin can safely read.
3.  **Message Parsing:** The `RailcomRx` library listens for this incoming serial data, validates it, and parses it into structured messages that your application can easily use.

## Wiring Diagram

This diagram shows how the XIAO RP2040 connects to a generic RailCom detector circuit. The design of the detector circuit itself is beyond the scope of this guide, but many open-source designs are available (e.g., from the RCN-Community).

**Connections:**
- The **output** of your RailCom detector circuit (a 3.3V logic-level UART signal) is connected to the XIAO's **UART RX pin (D7)**.
- The detector circuit and the XIAO must share a **common ground (GND)**.

```ascii
                      ┌───────────────────────────┐
DCC Signal from Track │                           │ Logic-Level UART Signal
─────────────────────►│    RailCom Detector       ├───────────────────► To XIAO RP2040 RX Pin
                      │        Circuit            │ (0 - 3.3V)
─────────────────────►│                           │
                      └───────────┬───────────────┘
                                  │
                                  ▼
                                 GND
                                  │
┌─────────────────────────────────┼─────────────────────────────────┐
│                                 │                                 │
│     ┌───────────────────────────┴───────────────────────────┐     │
│     │ D7/GPIO1 (UART0 RX)                                   │     │
│     │                                                       │     │
│     │                 Seeed Studio XIAO                     ├─────┤  GND
│     │                       RP2040                          │ GND │
│     │                                                       │     │
│     │                                                       │     │
│     └───────────────────────────────────────────────────────┘     │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

## Minimal Software Example

This example demonstrates how to initialize the `RailcomRx` library, listen for incoming messages, and print them to the Serial Monitor in a human-readable format.

**Code:**
```cpp
#include <Arduino.h>
#include "RailcomRx.h"
#include "RailcomDebug.h" // Provides the printRailcomMessage() helper

// 1. Initialize the RailcomRx library.
//    - uart0: The hardware UART instance to use.
//    - RX Pin (1 -> D7 on XIAO): The GPIO pin for the UART RX signal,
//      which receives data from the external detector circuit.
RailcomRx railcomRx(uart0, 1);

void setup() {
  // Start the RailCom receiver. This configures the UART to listen.
  railcomRx.begin();

  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for the serial port to connect.
  }
  Serial.println("RailCom RX Detector Example Initialized");
  Serial.println("Listening for messages...");
}

void loop() {
  // 2. The railcomRx.task() function must be called repeatedly in the main loop.
  //    It continuously checks the UART for incoming bytes and assembles
  //    them into complete messages.
  railcomRx.task();

  // 3. Check if a complete and valid RailCom message has been received.
  if (railcomRx.available()) {
    // 4. If a message is available, read it from the library's buffer.
    RailcomMsg msg = railcomRx.read();

    // 5. Print the message to the Serial Monitor for debugging.
    //    The printRailcomMessage() function is a helpful utility from
    //    RailcomDebug.h that formats the message content nicely.
    Serial.print("Received RailCom Message: ");
    printRailcomMessage(msg);
  }
}
```

**Explanation:**

The workflow for receiving is even simpler than transmitting:

1.  **Initialization:** In `setup()`, we create an instance of `RailcomRx`, telling it which UART and pin to listen on.
2.  **Polling for Data:** In the `loop()`, `railcomRx.task()` does the essential background work of processing raw serial data.
3.  **Checking for a Message:** We then call `railcomRx.available()`, which returns `true` only when a complete, checksum-verified RailCom message has been successfully parsed.
4.  **Reading the Message:** Once a message is available, we call `railcomRx.read()` to retrieve it. This returns a `RailcomMsg` struct, which contains the message ID, payload, and other details.
5.  **Using the Data:** In this example, we simply print the message using the `printRailcomMessage` helper. In a real command station, you would use a `switch` statement on `msg.id` to handle different message types and update your application's state accordingly (e.g., update a locomotive's known address).
