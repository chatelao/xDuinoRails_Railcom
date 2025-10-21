# RP2040 RailCom Library

An Arduino library for encoding and decoding RailCom messages on the RP2040, compliant with the RCN-217 specification.

## Features

-   **High-Level API:** `RailcomTxManager` and `RailcomRxManager` classes simplify creating, sending, and parsing RCN-217 messages.
-   **Robust Sending:** Uses a non-blocking, ISR-driven message queue to ensure correct timing.
-   **Decoder State Machine:** Includes a `DecoderStateMachine` class to demonstrate realistic response logic.
-   **PIO-based Cutout:** Uses the RP2040's PIO to generate the precise RailCom cutout.
-   **Comprehensive Examples:** Includes `Dummy` and `NmraDcc`-based examples for various decoder types.

## Requirements

This library depends on the **Raspberry Pi Pico/RP2040** board support package by Earle Philhower to enable advanced features like PIO.

1.  In the Arduino IDE, go to `File > Preferences`.
2.  Add the following URL to the "Additional Board Manager URLs" field:
    ```
    https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
    ```
3.  Go to `Tools > Board > Boards Manager...`, search for "pico", and install the **Raspberry Pi Pico/RP2040** package.

## Installation
1.  In the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...`
2.  Search for "RP2040 Railcom" and install.
3.  Also install the required dependencies: `AUnit` and `NmraDcc`.

## Getting Started

The library is split into a transmitter (`RailcomTxManager`) and receiver (`RailcomRxManager`).

### Example: Locomotive Decoder

```cpp
#include <Arduino.h>
#include "RailcomSender.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t LOCO_ADDRESS = 4098;

RailcomSender sender(uart0, 0, 1); // UART, TX Pin, PIO Pin
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

void setup() {
    sender.begin();
}

void loop() {
    sender.task(); // Must be called repeatedly

    // Simulate a DCC packet for our address
    DCCMessage dcc_msg;
    stateMachine.handleDccPacket(dcc_msg);
    sender.begin_cutout_sequence(dcc_msg);
    delay(1000);
}
```

## API Reference
For a detailed API reference, please see the **[API Documentation](docs/API_Reference.md)**.
