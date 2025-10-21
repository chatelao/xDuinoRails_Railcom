# RP2040 RailCom Library

An Arduino library for encoding and decoding RailCom messages from and to an RP2040 UART, compliant with the RCN-217 specification.

## Features

-   **High-Level RCN-217 API:** A `RailcomManager` class that simplifies the creation and sending of standard RailCom messages for both vehicle and accessory decoders.
-   **RailCom Encoding and Decoding:** Handles the 4-out-of-8 encoding and decoding required for robust communication.
-   **PIO-based Cutout:** Uses the RP2040's PIO to generate the precise cutout required for RailCom.
-   **Comprehensive Examples:** Includes examples for locomotive, accessory, and function decoders, as well as a command station sketch for testing.

## Installation

1.  Download the latest release from the [GitHub repository](https://github.com/Jules/rp2040-railcom).
2.  In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...` and select the downloaded file.

## Getting Started

The core of the library is the `RailcomManager` class. It provides a high-level interface for sending RCN-217 compliant messages.

### Example: Locomotive Decoder

This example shows how a simple locomotive decoder would broadcast its address.

```cpp
#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

// Use a hardware UART stream for communication
HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0); // PIO pin is TX pin
RailcomReceiver receiver(&stream);
RailcomManager manager(sender, receiver);

void setup() {
  sender.begin();
  receiver.begin();
}

void loop() {
  // A real decoder would send this in response to a DCC packet.
  manager.sendAddress(LOCOMOTIVE_ADDRESS);
  delay(1000);
}
```

For more detailed examples, see the `examples` folder:
- **LocomotiveDecoder:** Demonstrates address broadcasting and responding to POM requests.
- **AccessoryDecoder:** Shows how to report status and send Service Requests (SRQ).
- **FunctionDecoder:** Simulates reporting dynamic data like fuel levels.
- **CommandStation:** An interactive sketch to test your decoders.

## API Reference

For a detailed API reference and explanation of the high-level functions, please see the **[RCN-217 API Documentation](docs/RCN-217_API.md)**.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request.

## License

This project is licensed under the MIT License.
