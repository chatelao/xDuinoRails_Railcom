# RP2040 RailCom Library

An Arduino library for encoding and decoding RailCom messages on the RP2040, compliant with the RCN-217 specification.

## Features

-   **High-Level API:** `RailcomTx` (for decoders) and `RailcomRx` (for detectors/centrales) classes simplify creating, sending, and parsing RCN-217 messages.
-   **Robust Sending:** Uses a non-blocking, ISR-driven message queue to ensure correct timing.
-   **Decoder State Machine:** Includes a `DecoderStateMachine` class to demonstrate realistic response logic.
-   **PIO-based Cutout:** Uses the RP2040's PIO to generate the precise RailCom cutout.
-   **Comprehensive Examples:** Includes `Dummy` and `NmraDcc`-based examples for various decoder types.

## Real-World Use Cases

RailCom's bidirectional communication opens up many possibilities for realistic and automated layout control. Here are some of the most common applications:

1.  **Automatic Locomotive Recognition:** A command station can instantly identify a locomotive when you place it on the track, eliminating the need to manually enter its address.
2.  **On-the-Main Programming:** Read and write decoder CVs on the main track without needing a separate programming track, making it much faster to fine-tune performance.
3.  **Accessory Feedback:** Turnout decoders can report their actual position back, ensuring your control panel is always synchronized with the physical layout.

For a more comprehensive list of what you can do with RailCom, see the **[full Use Cases documentation](docs/UseCases.md)**.

## Neopixel Example

The `LocomotiveDecoderNeopixel` example requires the [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) library to be installed. You can install it through the Arduino Library Manager.

## Installation
1.  In the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...`
2.  Search for "RP2040 Railcom" and install.
3.  Also install the required dependencies: `AUnit` and `NmraDcc`.

## Getting Started

To get started, please see the detailed "howto" guides for wiring and minimal software examples:

-   **[Howto: RailCom Decoder (Transmitter)](docs/HowtoTx.md)**
-   **[Howto: RailCom Detector (Receiver)](docs/HowtoRx.md)**

## API Reference
For a detailed API reference, please see the **[API Documentation](docs/API_Reference.md)**.

## Testing
For a detailed overview of the testing strategy, please see the **[Testing Documentation](docs/Testing.md)**.
