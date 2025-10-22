# API Reference

This document provides a detailed overview of the classes used to send and receive RCN-217 compliant RailCom messages.

## `RailcomTx` Class

This class provides the interface for queueing and sending RailCom messages.

### `RailcomTx(uart_inst_t* uart, uint tx_pin, uint pio_pin)`
Constructs a new transmitter.

### `sendPomResponse(uint8_t cvValue)`
Queues a `POM` (ID 0) response, typically after a POM read request.

### `sendAddress(uint16_t address)`
Queues an `ADR` (ID 1/2) broadcast. Correctly handles alternating between the high and low parts for long addresses.

### `sendDynamicData(uint8_t subIndex, uint8_t value)`
Queues a `DYN` (ID 7) message to report dynamic data.

---

## `RailcomRx` Class

This class provides the interface for parsing incoming RailCom byte streams.

### `RailcomRx(uart_inst_t* uart, uint rx_pin)`
Constructs a new receiver.

### `RailcomMessage* readMessage()`
The main parser function. This should be called repeatedly in your `loop()`. It processes incoming bytes and returns a pointer to a `RailcomMessage` struct if a complete message has been successfully parsed. Returns `nullptr` if no message is ready.

You can then `static_cast` the result to the appropriate message type based on the `id` field.

```cpp
RailcomMessage* msg = railcomRx.readMessage();
if (msg != nullptr) {
    if (msg->id == RailcomID::POM) {
        PomMessage* pomMsg = static_cast<PomMessage*>(msg);
        Serial.println(pomMsg->cvValue);
    }
}
```
