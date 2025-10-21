# API Reference

This document provides a detailed overview of the high-level classes used to send and receive RCN-217 compliant RailCom messages.

## `RailcomTxManager` Class

This class provides the high-level interface for queueing RailCom messages to be sent.

### `RailcomTxManager(RailcomSender& sender)`
Constructs a new manager. It requires a reference to a low-level `RailcomSender` instance.

### `sendPomResponse(uint8_t cvValue)`
Queues a `POM` (ID 0) response, typically after a POM read request.

### `sendAddress(uint16_t address)`
Queues an `ADR` (ID 1/2) broadcast. Correctly handles alternating between the high and low parts for long addresses.

### `sendDynamicData(uint8_t subIndex, uint8_t value)`
Queues a `DYN` (ID 7) message to report dynamic data.

---

## `RailcomRxManager` Class

This class provides the high-level interface for parsing incoming RailCom byte streams.

### `RailcomRxManager(RailcomReceiver& receiver)`
Constructs a new manager. It requires a reference to a low-level `RailcomReceiver` instance.

### `RailcomMessage* readMessage()`
The main parser function. This should be called repeatedly in your `loop()`. It processes incoming bytes and returns a pointer to a `RailcomMessage` struct if a complete message has been successfully parsed. Returns `nullptr` if no message is ready.

You can then `static_cast` the result to the appropriate message type based on the `id` field.

```cpp
RailcomMessage* msg = rxManager.readMessage();
if (msg != nullptr) {
    if (msg->id == RailcomID::POM) {
        PomMessage* pomMsg = static_cast<PomMessage*>(msg);
        Serial.println(pomMsg->cvValue);
    }
}
```
