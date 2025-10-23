# API Reference

This document provides a detailed reference to the public API of the RailCom library.

## `RailcomTx`

The `RailcomTx` class is responsible for transmitting RailCom messages. It uses the RP2040's PIO to generate the precise timing required for the RailCom cutout.

### `RailcomTx(uart_inst_t* uart, uint tx_pin, uint pio_pin)`

Constructor for the `RailcomTx` class.

- **`uart`**: The UART instance to use for transmitting the RailCom signal (e.g., `uart0`).
- **`tx_pin`**: The GPIO pin to use for the UART TX signal.
- **`pio_pin`**: The GPIO pin connected to the DCC signal, used by the PIO to detect the cutout.

### `void begin()`

Initializes the PIO and UART for RailCom transmission. This must be called before any other methods.

### `void end()`

Deinitializes the PIO and UART.

### `void task()`

This method must be called repeatedly in the main application loop. It handles the non-blocking sending of queued messages.

### `void send_dcc_with_cutout(const DCCMessage& dccMsg)`

Sends a DCC message and immediately triggers the PIO cutout sequence for sending any queued RailCom messages.

### Vehicle Decoder (MOB) Functions

- **`void sendPomResponse(uint8_t cvValue)`**: Sends a POM response with the value of a single CV.
- **`void sendAddress(uint16_t address)`**: Sends the decoder's address.
- **`void sendDynamicData(uint8_t subIndex, uint8_t value)`**: Sends a dynamic data message.
- **`void sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4])`**: Sends a response for an indexed POM write.
- **`void handleRerailingSearch(uint16_t address, uint32_t secondsSincePowerOn)`**: Handles the rerailing search sequence.

### Accessory Decoder (STAT) Functions

- **`void sendServiceRequest(uint16_t accessoryAddress, bool isExtended)`**: Sends a service request for an accessory decoder.
- **`void sendStatus1(uint8_t status)`**: Sends a status message (type 1).
- **`void sendStatus4(uint8_t status)`**: Sends a status message (type 4).
- **`void sendError(uint8_t errorCode)`**: Sends an error message.

### RCN-218 DCC-A Functions

- **`void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId)`**: Sends the decoder's unique ID.
- **`void sendDecoderState(uint8_t changeFlags, uint16_t changeCount, uint16_t protocolCaps)`**: Sends the decoder's state.
- **`void sendDataSpace(const uint8_t* data, size_t len, uint8_t dataSpaceNum)`**: Sends data to a specific data space.
- **`void sendAck()`**: Sends an ACK.
- **`void sendNack()`**: Sends a NACK.

## `RailcomRx`

The `RailcomRx` class is responsible for receiving RailCom messages.

### `RailcomRx(uart_inst_t* uart, uint rx_pin)`

Constructor for the `RailcomRx` class.

- **`uart`**: The UART instance to use for receiving the RailCom signal (e.g., `uart0`).
- **`rx_pin`**: The GPIO pin to use for the UART RX signal.

### `void begin()`

Initializes the UART for RailCom reception.

### `void end()`

Deinitializes the UART.

### `void set_decoder_address(uint16_t address)`

Sets the address of the decoder being communicated with. This is used for filtering messages.

### `RailcomMessage* readMessage()`

Reads and parses a RailCom message from the UART. Returns a pointer to a `RailcomMessage` object if a message was successfully parsed, otherwise returns `nullptr`. The type of the returned message can be determined by its `id` field.

## `DecoderStateMachine`

The `DecoderStateMachine` class encapsulates the logic for deciding which RailCom message to send in response to a given DCC packet.

### `DecoderStateMachine(RailcomTxManager& txManager, DecoderType type, uint16_t address, uint16_t manufacturerId = 0, uint32_t productId = 0)`

Constructor for the `DecoderStateMachine`.

- **`txManager`**: A reference to the `RailcomTx` instance.
- **`type`**: The type of the decoder (`LOCOMOTIVE` or `ACCESSORY`).
- **`address`**: The address of the decoder.
- **`manufacturerId`**: (Optional) The manufacturer ID of the decoder.
- **`productId`**: (Optional) The product ID of the decoder.

### `void handleDccPacket(const DCCMessage& dccMsg)`

This is the main entry point for the state machine. It analyzes the incoming DCC packet and queues the appropriate RailCom response using the `RailcomTx` instance.

## `Railcom`

The `Railcom` namespace and data structures provide common definitions and utility functions used throughout the library.

### `DCCMessage`

A class that encapsulates a DCC message.

- **`DCCMessage(const uint8_t* data, size_t len)`**: Constructor to create a DCC message from a byte array.
- **`const uint8_t* getData() const`**: Returns a pointer to the raw DCC message data.
- **`size_t getLength() const`**: Returns the length of the DCC message in bytes.

### `RailcomID`

An enum that defines the various RailCom message IDs, as specified in RCN-217 and RCN-218.

### `RailcomMessage`

A base struct for all RailCom messages. It contains a `RailcomID` field.

### `Railcom::encode4of8(uint8_t value)`

Encodes a 6-bit value into a 4-out-of-8 encoded byte.

### `Railcom::decode4of8(uint8_t encodedByte)`

Decodes a 4-out-of-8 encoded byte back into a 6-bit value.

### `Railcom::crc8(const uint8_t* data, size_t len, uint8_t init = 0)`

Calculates the RCN-218 CRC8 checksum.
