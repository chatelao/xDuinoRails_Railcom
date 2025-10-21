# RP2040 RailCom Library

An Arduino library for encoding and decoding RailCom messages from and to an RP2040 UART.

## Features

-   **RailCom Encoding and Decoding:**  Handles the low-level details of RailCom communication.
-   **DCC Message Parsing:**  Parses raw DCC messages to extract commands and addresses.
-   **Automatic Locomotive Detection:**  Implements the RailCom discovery protocol.
-   **PIO-based Cutout:**  Uses the RP2040's PIO to generate the precise cutout required for RailCom.
-   **Separate Sender and Receiver Classes:** Provides a clear and easy-to-use API for sending and receiving RailCom messages.

## Installation

1.  Download the latest release from the [GitHub repository](https://github.com/your-username/rp2040-railcom).
2.  In the Arduino IDE, go to `Sketch > Include Library > Add .ZIP Library...` and select the downloaded file.

## Usage

### RailcomSender

```cpp
#include <Arduino.h>
#include "Railcom.h"

RailcomSender sender(uart0, 0, 1);

void setup() {
  Serial.begin(115200);
  sender.begin();
}

void loop() {
  // Send a DCC message with a RailCom cutout
  uint8_t dcc_data[] = {0xFF, 0xFF, 0x01};
  DCCMessage msg(dcc_data, sizeof(dcc_data));
  sender.send_dcc_async(msg);
  delay(100);
}
```

### RailcomReceiver

```cpp
#include <Arduino.h>
#include "Railcom.h"

RailcomReceiver receiver(uart1, 4, 5);

void setup() {
  Serial.begin(115200);
  receiver.begin();
  receiver.set_decoder_address(0x1234);
}

void loop() {
  // Read incoming DCC messages
  // In a real application, this would come from a DCC decoder
    uint8_t dcc_data[] = {0xFF, 0xFF, 0x01};
    DCCMessage msg = receiver.parse_dcc_message(dcc_data, sizeof(dcc_data));

  // Handle the DCC message
  receiver.handle_dcc_message(msg);

  // Check for discovered locomotives
  const std::vector<uint16_t>& addresses = receiver.get_discovered_addresses();
  for (uint16_t address : addresses) {
    Serial.print("Discovered locomotive: ");
    Serial.println(address, HEX);
  }
}
```

## API

### `DCCMessage`

#### `DCCMessage(const uint8_t* data, size_t len)`

Creates a new DCC message.

#### `uint16_t getAddress() const`

Gets the address of the DCC message.

#### `uint8_t getCommand() const`

Gets the command of the DCC message.

### `RailcomSender`

#### `RailcomSender(uart_inst_t* uart, uint txPin, uint rxPin)`

Creates a new RailCom sender.

#### `void begin()`

Initializes the UART and PIO.

#### `void end()`

Deinitializes the UART and PIO.

#### `void send_dcc_async(const DCCMessage& msg)`

Sends a DCC message with a RailCom cutout.

### `RailcomReceiver`

#### `RailcomReceiver(uart_inst_t* uart, uint txPin, uint rxPin)`

Creates a new RailCom receiver.

#### `void begin()`

Initializes the UART.

#### `void end()`

Deinitializes the UART.

#### `bool read_response(uint8_t* buffer, size_t len, uint timeout_ms)`

Reads a RailCom response.

#### `DCCMessage parse_dcc_message(const uint8_t* data, size_t len)`

Parses a raw DCC message.

#### `void set_decoder_address(uint16_t address)`

Sets the address of the decoder.

#### `void handle_dcc_message(const DCCMessage& msg)`

Handles a DCC message, including discovery requests.

#### `const std::vector<uint16_t>& get_discovered_addresses() const`

Gets the addresses of discovered locomotives.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request.

## License

This project is licensed under the MIT License.
