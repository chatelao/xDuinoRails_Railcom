# RCN-217 Implementation Analysis Report

This document details the implementation status of features from the RCN-217 specification within the RP2040 RailCom Arduino library.

---

## Mobile Decoder Features (MOB)

*   **`ADR_HIGH` / `ADR_LOW` (IDs 1 & 2) - Address Reporting**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendAddress` handles both long and short addresses correctly, alternating between high and low parts.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses both message types. The `print()` function combines them to show the effective address.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino` (`short_address_e2e`, `long_address_e2e`).

*   **`POM` (ID 0) - Programming on the Main**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendPomResponse` sends the 8-bit CV value.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the response.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`DYN` (ID 7) - Dynamic Data**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendDynamicData` sends a message with a sub-index and value.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`XPOM` (IDs 8, 9, 10, 11) - Extended Programming on the Main**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendXpomResponse` sends a 32-bit payload with four CV values.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses all four sequence-numbered `XPOM` messages.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`CV-AUTO` (ID 12) - Automatic CV Transmission**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendCvAuto` sends a 24-bit CV address and 8-bit value.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`Aufgleissuche` / Rerailing Search (ID 14)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::handleRerailingSearch` sends the required sequence of `ADR_HIGH`, `ADR_LOW`, and `RERAIL` messages.
    *   **Rx:** `RailcomRx::parseMessage` can parse all individual messages of the sequence.
    *   **Test:** The full sequence is tested end-to-end in `tests/RailcomTest/rerailing_search_e2e`. The library does not implement the DCC parsing to trigger this automatically from a broadcast command.

*   **`EXT` (ID 3) - Location Information**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendExt` correctly encodes and sends the 14-bit payload with type and position.
    *   **Rx:** `RailcomRx::parseMessage` correctly differentiates `EXT` from `STAT4` by length and fully parses the 14-bit payload.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`INFO1` (ID 3) - Additional Info**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx` can be configured via `enableInfo1()` to cyclically send the INFO1 message on Channel 1 as part of the address broadcast.
    *   **Rx:** `RailcomRx` uses a decoder context (`setContext`) to correctly distinguish and parse INFO1 messages from STAT4 messages, as they share the same ID and length.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino` (`info1_cycle_e2e`).

*   **`Aktuelle Fahrinformation` / Current Driving Info (ID 4)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendInfo` sends a 32-bit payload containing speed, motor load, and status flags.
    *   **Rx:** `RailcomRx::parseMessage` distinguishes INFO from STAT1 by message length and decoder context, then parses the 32-bit payload.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/info_message_e2e`.

*   **`BLOCK` (ID 13)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendBlock` sends a 32-bit data payload.
    *   **Rx:** `RailcomRx::parseMessage` distinguishes `BLOCK` (32-bit payload) from `DECODER_STATE` (44-bit payload) based on the total message length.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

## Stationary Decoder Features (STAT)

*   **`STAT1` (ID 4) - Status Report Part 1**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendStatus1` sends the status byte.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`STAT4` (ID 3) - Status of 4 Turnout Pairs**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendStatus4` sends the status byte.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message and can distinguish it from `EXT`.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`FEHLER` / `ERROR` (ID 6)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendError` sends the error code.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`ZEIT` / `TIME` (ID 5)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendTime` sends the time value and unit.
    *   **Rx:** `RailcomRx::parseMessage` correctly parses the message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`STAT2` (ID 8)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendStatus2` sends the status byte.
    *   **Rx:** `RailcomRx::parseMessage` correctly distinguishes and parses the short `STAT2` message from the long `XPOM` message.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/RailcomTest.ino`.

*   **`SRQ` - Service Request (ID 14)**
    *   **Status: Implemented**
    *   **Tx:** `RailcomTx::sendServiceRequest` correctly encodes the 11-bit address and extended flag.
    *   **Rx:** `RailcomRx::parseMessage` correctly distinguishes `SRQ` from `RERAIL` based on length and parses the payload.
    *   **Test:** Verified end-to-end in `tests/RailcomTest/service_request_e2e`.

## Protocol-Level Features

*   **4-of-8 Data Encoding**
    *   **Status: Implemented**
    *   **Details:** `RailcomEncoding.cpp` contains the complete encoding and decoding logic used by all message types.

*   **Datagram Packet Structure**
    *   **Status: Implemented**
    *   **Details:** The library correctly constructs and parses datagrams with a 4-bit ID and payload.

*   **CV-Based Configuration (CV28/CV29)**
    *   **Status: Implemented**
    *   **Details:** The `DecoderStateMachine` uses CV29 to enable or disable RailCom transmissions.
    *   **Test:** Verified in `tests/RailcomTest/cv_config_disables_railcom`.

*   **Dynamic Channel 1 Usage**
    *   **Status: Implemented**
    *   **Details:** The `DecoderStateMachine` stops broadcasting its address on Channel 1 after being directly addressed.
    *   **Test:** Verified in `tests/RailcomTest/dynamic_channel1_management`.

*   **Handling of Specific DCC Commands (XF1, XF2, etc.)**
    *   **Status: Partially Implemented (XF1)**
    *   **Details:** The `DecoderStateMachine` now automatically responds to an `XF1` (request for location information) DCC command by sending an `EXT` message on RailCom Channel 2. The DCC parsing logic for other extended function commands is not yet implemented.

---
