# API Reference

This document provides a detailed reference for the public API of the RP2040 RailCom Library. The library is structured around a Hardware Abstraction Layer (HAL) and high-level manager classes.

## High-Level Classes

These are the main classes you will interact with in your application.

### `RailcomTx`

Manages the queuing and transmission of RailCom messages. Designed for use in a decoder.

- **`RailcomTx(RailcomTxHardware* hardware)`**: Constructor. Takes a pointer to a concrete hardware implementation (e.g., `RP2040RailcomTxHardware`).
- **`void begin()`**: Initializes the transmitter.
- **`void on_cutout_start(uint32_t elapsed_us = 0)`**: Triggers the sending of queued messages. This should be called at the start of the DCC cutout.
- **`void sendPomResponse(uint8_t cvValue)`**: Queues a POM response on Channel 2.
- **`void sendAddress(uint16_t address)`**: Manages the broadcast of the decoder's address on Channel 1. Handles alternating between `ADR_HIGH` and `ADR_LOW` for long addresses.
- **`void enableInfo1(const Info1Message& info1)`**: Includes an `INFO1` message in the Channel 1 address broadcast cycle.
- **`void disableInfo1()`**: Removes the `INFO1` message from the Channel 1 broadcast cycle.
- **`void sendServiceRequest(uint16_t accessoryAddress, bool isExtended)`**: Queues a service request (SRQ) for an accessory decoder on Channel 2.
- **`void sendDecoderUnique(uint16_t manufacturerId, uint32_t productId)`**: Queues the decoder's unique ID (RCN-218) on Channel 2.
- **`void sendDecoderState(...)`**: Queues the decoder's state (RCN-218) on Channel 2.
- **`void sendDataSpace(...)`**: Queues a raw Data Space message (RCN-218) on Channel 2.
- _...and many other `send...` methods for all RCN-217 and RCN-218 message types._

### `RailcomRx`

Manages the reception and parsing of RailCom messages. Designed for use in a command station or detector.

- **`RailcomRx(RailcomRxHardware* hardware)`**: Constructor. Takes a pointer to a concrete hardware implementation (e.g., `RP2040RailcomRxHardware`).
- **`void begin()`**: Initializes the receiver.
- **`void task()`**: A periodic task function to be called in the main loop.
- **`RailcomMessage* read()`**: Reads, decodes, and parses a message from the hardware. Returns a pointer to a base `RailcomMessage` struct. The caller must cast this to the appropriate message type based on the `id` field. Returns `nullptr` if no valid message is available.
- **`void setContext(DecoderContext context)`**: Sets the context (e.g., `MOBILE` or `STATIONARY`) to disambiguate messages with shared IDs.
- **`void print(Print& stream)`**: Prints a human-readable summary of the last received message to a stream (e.g., `Serial`).
- **`void expectDataSpaceResponse(uint8_t dataSpaceNum)`**: Flags the receiver to parse the next message as a special RCN-218 Data Space response.

### `DecoderStateMachine`

A high-level class that encapsulates the logic of a decoder, linking DCC packet parsing to RailCom responses.

- **`DecoderStateMachine(RailcomTx& txManager, ...)`**: Constructor. Takes a reference to `RailcomTx` and various decoder configuration parameters (type, address, CVs, etc.).
- **`void handleDccPacket(const DCCMessage& dccMsg)`**: The main entry point. Analyzes an incoming `DCCMessage` and triggers the appropriate RailCom response.
- **`void task()`**: A periodic task function for handling background processes like the automatic CV broadcast.

### `RailcomDccParser`

A callback-based parser for DCC commands relevant to RailCom. Used internally by `DecoderStateMachine`.

- **`void parse(const DCCMessage& msg, bool* response_sent = nullptr)`**: Parses a `DCCMessage` and invokes any matching registered callback.
- **`std::function<void(...)> on...`**: Numerous public `std::function` members that can be assigned callbacks for specific DCC events (e.g., `onPomReadCv`, `onLogonEnable`, `onAccessory`).

## Hardware Abstraction Layer (HAL)

These abstract base classes define the interface between the high-level logic and the specific hardware platform.

### `RailcomTxHardware`

- **`virtual void begin() = 0`**: Initializes the hardware.
- **`virtual void send_bytes(const std::vector<uint8_t>& bytes) = 0`**: Sends a vector of raw, pre-encoded bytes.

### `RailcomRxHardware`

- **`virtual void begin() = 0`**: Initializes the hardware.
- **`virtual int available() = 0`**: Returns the number of bytes available to read.
- **`virtual int read() = 0`**: Reads a single byte from the hardware.

## RP2040 HAL Implementations

These are the concrete implementations of the HAL for the RP2040 microcontroller.

### `RP2040RailcomTxHardware`

- **`RP2040RailcomTxHardware(uart_inst_t* uart, uint tx_pin)`**: Constructor.
  - `uart`: The RP2040 UART instance (e.g., `uart0`).
  - `tx_pin`: The GPIO pin number for UART TX.

### `RP2040RailcomRxHardware`

- **`RP2040RailcomRxHardware(uart_inst_t* uart, uint rx_pin)`**: Constructor.
  - `uart`: The RP2040 UART instance (e.g., `uart0`).
  - `rx_pin`: The GPIO pin number for UART RX.

## Core Data Structures (`Railcom.h`)

- **`class DCCMessage`**: Encapsulates a raw DCC packet (data pointer and length).
- **`enum class RailcomID`**: Defines all message IDs from RCN-217 and RCN-218.
- **`struct RailcomMessage`**: The base struct for all parsed messages. Contains the `id`.
- **`struct PomMessage`, `struct AdrMessage`, etc.**: Specific message structs that inherit from `RailcomMessage` and contain the decoded payload data for each message type.
