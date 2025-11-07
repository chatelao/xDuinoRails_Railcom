# RCN-217 Implementation Analysis Report

This document details the implementation status of features from the RCN-217 specification within the RP2040 RailCom Arduino library.

---

## Mobile Decoder Features (MOB)

*   **`ADR_HIGH` / `ADR_LOW` (IDs 1 & 2) - Address Reporting**
    *   **Status: Implemented**
    *   **Details:** The library can correctly send and parse both high and low address parts for short and long addresses. This is demonstrated in `tests/RailcomTest/RailcomTest.ino` and used in the `examples/LocomotiveDecoderNmra/LocomotiveDecoderNmra.ino` sketch via the `DecoderStateMachine`.

*   **`POM` (ID 0) - Programming on the Main**
    *   **Status: Implemented**
    *   **Details:** The library supports sending and receiving POM responses for CV values. This is tested end-to-end in `tests/RailcomTest/RailcomTest.ino` and used in the locomotive example.

*   **`DYN` (ID 7) - Dynamic Data**
    *   **Status: Implemented**
    *   **Details:** The library can send and parse `DYN` messages containing a sub-index and value. This is verified in `tests/RailcomTest/RailcomTest.ino`. However, the specific meanings of most sub-indexes (e.g., speed, load) are not implemented in the examples.

*   **`XPOM` (IDs 8, 9, 10, 11) - Extended Programming on the Main**
    *   **Status: Implemented**
    *   **Details:** The library can send and parse all four sequence-numbered `XPOM` messages, which carry four CV values. This is verified in `tests/RailcomTest/RailcomTest.ino`.

*   **`CV-AUTO` (ID 12) - Automatic CV Transmission**
    *   **Status: Implemented**
    *   **Details:** The library can send and parse `CV-AUTO` messages containing a 24-bit CV address and an 8-bit value. This is verified in `tests/RailcomTest/RailcomTest.ino`.

*   **`Aufgleissuche` / Rerailing Search (ID 14)**
    *   **Status: Partially Implemented**
    *   **Details:** The `RailcomTx` class has a `handleRerailingSearch` method to send the required `ADR_HIGH`, `ADR_LOW`, and `RERAIL` (ID 14) messages. However, no example or test sketch actually calls this function, and there is no logic to handle the triggering broadcast command (`XF2 aus`).

*   **`INFO1` (ID 3) - Additional Info for Channel 1**
    *   **Status: Not Implemented**
    *   **Details:** There is no definition, sender function, or parsing logic for the `INFO1` message, which is used to report direction and state on Channel 1.

*   **`EXT` (ID 3) - Location Information**
    *   **Status: Not Implemented**
    *   **Details:** The library does not implement the `EXT` message for transmitting location information. The message ID `3` is used for `STAT4` in the stationary decoder context.

*   **`Aktuelle Fahrinformation` / Current Driving Info (ID 4)**
    *   **Status: Not Implemented**
    *   **Details:** The 36-bit message for detailed driving information is not implemented. The message ID `4` is used for `STAT1`.

*   **`BLOCK` (ID 13)**
    *   **Status: Not Implemented**
    *   **Details:** There is no support for this message type. (Note: The library uses ID 13 for `DECODER_STATE` as defined in the RCN-218 specification).

## Stationary Decoder Features (STAT)

*   **`STAT1` (ID 4) - Status Report Part 1**
    *   **Status: Implemented**
    *   **Details:** The library can send and parse `STAT1` messages. This is tested in `tests/RailcomTest/RailcomTest.ino` and used in the `examples/AccessoryDecoderNmra/AccessoryDecoderNmra.ino` sketch.

*   **`STAT4` (ID 3) - Status of 4 Turnout Pairs**
    *   **Status: Implemented**
    *   **Details:** The library can send and parse `STAT4` messages. This is also tested and used in the accessory decoder example.

*   **`FEHLER` / `ERROR` (ID 6)**
    *   **Status: Implemented**
    *   **Details:** The library supports sending and parsing the `ERROR` message. It is verified in the test suite.

*   **`ZEIT` / `TIME` (ID 5)**
    *   **Status: Implemented**
    *   **Details:** The library supports sending and parsing the `TIME` message for reporting estimated switching time. It is verified in the test suite.

*   **`STAT2` (ID 8)**
    *   **Status: Implemented**
    *   **Details:** The library can correctly distinguish and parse the short `STAT2` message from the long `XPOM` message, both of which use ID 8. This is verified in the test suite.

*   **`SRQ` - Service Request (Channel 1)**
    *   **Status: Partially Implemented**
    *   **Details:** The `RailcomTx` class has a `sendServiceRequest` function, and the encoding logic is present. However, none of the example sketches demonstrate its use, and there is no logic to handle the required NOP packets or search algorithms for multiple SRQs.

## Protocol-Level Features

*   **4-of-8 Data Encoding**
    *   **Status: Implemented**
    *   **Details:** The `RailcomEncoding.cpp` file contains the complete encoding and decoding lookup tables and functions, which are used by all message types.

*   **Datagram Packet Structure**
    *   **Status: Implemented**
    *   **Details:** The library correctly constructs datagrams by combining a 4-bit message ID with a variable-length payload, as shown in `RailcomEncoding::encodeDatagram`.

*   **CV-Based Configuration (CV28/CV29)**
    *   **Status: Not Implemented**
    *   **Details:** The library does not contain logic to read CVs like CV28 to dynamically enable or disable RailCom channels or features. The behavior is currently hardcoded.

*   **Dynamic Channel 1 Usage**
    *   **Status: Not Implemented**
    *   **Details:** The logic for a decoder to automatically stop sending its address on Channel 1 after being addressed several times (to reduce channel collisions) is not implemented.

*   **Handling of Specific DCC Commands (XF1, XF2, etc.)**
    *   **Status: Not Implemented**
    *   **Details:** The library provides the functions to send RailCom messages *in response* to events, but it does not include the DCC parsing logic to recognize and react to the specific extended function commands (like `XF1` for location request or `XF2` for rerailing search) that trigger these responses.
