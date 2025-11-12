# RP2040 RailCom Library

[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/11454/badge)](https://www.bestpractices.dev/projects/11454)

An Arduino library for encoding and decoding RailCom messages on the RP2040, compliant with the RCN-217 specification.

## Architecture and Scope

This project provides a library for RailCom® communication for digital model railways. The library is divided into two main components with clearly defined roles:

### RailcomRx (Receiver)

-   **Scope:** The `RailcomRx` class is intended for use in a **command station** or a dedicated **RailCom detector**.
-   **Tasks:** Its main task is to receive and parse RailCom messages sent by decoders on the track.
-   **Limitations:** `RailcomRx` is **not** responsible for generating the DCC signal or the "cutout" (the powerless gap in the DCC signal) required for RailCom transmission. These tasks must be handled by the parent application (e.g., the command station's firmware). `RailcomRx` expects to receive the corresponding DCC message in the format of the `NmraDcc` library to understand the context of the received RailCom message.

### RailcomTx (Transmitter)

-   **Scope:** The `RailcomTx` class is designed for use in a **vehicle or accessory decoder**.
-   **Tasks:** Its sole task is to prepare and send RailCom response messages.
-   **Limitations:** `RailcomTx` is **not** responsible for decoding DCC signals, motor control, or controlling functions and other decoder features. The decoder's application logic must provide `RailcomTx` with the last received DCC message and the exact start time of the cutout. The library then sends the prepared RailCom messages.

```
DCC Command Station (Microcontroller)    Decoder (Microcontroller)
+----------------------+                 +--------------------+
|                      |                 |                    |
|    DCC_TX_PIN ------->----------------->------ DCC_RX_PIN   |
|         (DCC Signal) |                 |    (DCC Signal)    |
|                      |                 |                    |
| RC_CUTOUT_PIN ------->-------          |                    |
| RC_RX_PIN ===========<=================<====== RC_TX_PIN    |
|    (RailCom Signal)  |                 | (RailCom Signal)   |
|                      |                 |                    |
+----------------------+                 +--------------------+
```

## Features

-   **High-Level API:** `RailcomTx` (for decoders) and `RailcomRx` (for detectors/command stations) classes simplify the creation, sending, and parsing of RCN-217 messages.
-   **Asynchronous Sending:** Uses a message queue to prepare messages and send them at the correct time (during the cutout).
-   **Decoder State Machine:** Includes a `DecoderStateMachine` class to demonstrate realistic response logic for decoders.
-   **Comprehensive Examples:** Includes `Dummy` and `NmraDcc`-based examples for various decoder types.
-   **Web-based Tool:** A [web-based tool](https://chatelao.github.io/xDuinoRails_Railcom/tools/index.html) for decoding RailCom messages.

## Real-World Use Cases

RailCom's bidirectional communication opens up many possibilities for realistic and automated layout control. Here are some of the most common applications:

1.  **Automatic Locomotive Recognition:** A command station can instantly identify a locomotive when you place it on the track, eliminating the need to manually enter its address.
2.  **On-the-Main Programming:** Read and write decoder CVs on the main track without needing a separate programming track, making it much faster to fine-tune performance.
3.  **Accessory Feedback:** Turnout decoders can report their actual position back, ensuring your control panel is always synchronized with the physical layout.

For a more comprehensive list of what you can do with RailCom, see the **[full Use Cases documentation](docs/USE_CASES.md)**.

## Neopixel Example

The `LocomotiveDecoderNeopixel` example requires the [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) library to be installed. You can install it through the Arduino Library Manager.

## Installation
1.  In the Arduino IDE, go to `Sketch > Include Library > Manage Libraries...`
2.  Search for "RP2040 Railcom" and install.
3.  Also install the required dependencies: `AUnit` and `NmraDcc`.

## Getting Started

To get started, please see the detailed "howto" guides for wiring and minimal software examples:

-   **[Howto: RailCom Decoder (Transmitter)](docs/HOWTO_TX.md)**
-   **[Howto: RailCom Detector (Receiver)](docs/HOWTO_RX.md)**

## API Reference
For a detailed API reference, please see the **[API Documentation](docs/API_REFERENCE.md)**.

## Testing
For a detailed overview of the testing strategy, please see the **[Testing Documentation](docs/TESTING.md)**.

## Tools

### Railcom Bit Decoder

A simple web-based tool for decoding RailCom messages. You can paste hexadecimal or binary values and see the decoded 6-bit values and the full payload.

**[Try it here!](https://chatelao.github.io/xDuinoRails_Railcom/tools/index.html)**

## Contributing
Contributions are welcome! Please open an issue or submit a pull request on the [GitHub repository](https://github.com/chatelao/xDuinoRails_Railcom).
