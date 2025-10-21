# RCN-217 High-Level API Reference

This document provides a detailed overview of the `RailcomManager` class, which is the primary interface for sending RCN-217 compliant RailCom messages.

## `RailcomManager` Class

The `RailcomManager` simplifies the process of creating and sending RailCom datagrams. You create an instance by passing it references to a `RailcomSender` and a `RailcomReceiver`.

```cpp
#include "Railcom.h"
#include "RailcomManager.h"

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 0, 1);
RailcomManager manager(sender, receiver);
```

---

## Vehicle Decoder (MOB) Functions

These functions are used by mobile decoders, such as locomotives and function decoders.

### `sendAddress(uint16_t address)`
Broadcasts the decoder's address on Channel 1. The library correctly handles both short (1-127) and long (128+) addresses, sending the appropriate `ADR_HIGH` (ID 1) and `ADR_LOW` (ID 2) datagrams.

```cpp
// Broadcast a long address
manager.sendAddress(4098);
```

### `sendPomResponse(uint8_t cvValue)`
Sends a `POM` (ID 0) response. This is typically done after receiving a POM read request from a command station. The payload is the 8-bit value of the requested CV.

```cpp
// Respond to a read request for a CV with value 151
manager.sendPomResponse(151);
```

### `sendXpomResponse(uint8_t sequence, const uint8_t cvValues[4])`
Sends an `XPOM` (ID 8-11) response. This is used for reading or writing up to four CVs at once. The `sequence` number (0-3) must match the sequence number of the XPOM request.

```cpp
uint8_t values[] = {10, 20, 30, 40};
// Respond to the first XPOM request in a sequence
manager.sendXpomResponse(0, values);
```

### `sendDynamicData(uint8_t subIndex, uint8_t value)`
Sends a `DYN` (ID 7) message to report dynamic data, such as speed, temperature, or fuel level. The `subIndex` identifies the type of data being sent (see RCN-217 for a full list).

```cpp
// Report that fuel level (subIndex 5) is at 75%
manager.sendDynamicData(5, 75);
```

---

## Accessory Decoder (STAT) Functions

These functions are used by stationary decoders, such as turnout or signal decoders.

### `sendServiceRequest(uint16_t accessoryAddress, bool isExtended)`
Sends a Service Request (`SRQ`) on Channel 1. This is used to notify the command station that the decoder needs attention (e.g., due to a manual state change). This message has no ID.

```cpp
// Send an SRQ for a simple accessory decoder at address 100
manager.sendServiceRequest(100, false);
```

### `sendStatus1(uint8_t status)` and `sendStatus4(uint8_t status)`
Sends a status report. `STAT1` (ID 4) and `STAT4` (ID 3) are used to report the state of the accessory's outputs.

```cpp
// Report a status for a simple turnout (e.g., 1 = thrown)
manager.sendStatus1(1);
```

### `sendError(uint8_t errorCode)`
Sends an `ERROR` (ID 6) message to report a problem, such as a defective fuse or a command that could not be executed.

```cpp
// Report a "Fuse defective" error (code 0x01)
manager.sendError(0x01);
```
