# RailCom Message Types and Their Triggers (according to RCN-217)

This table summarizes the various RailCom messages defined in the RCN-217 specification and the conditions under which they are sent.

## Vehicle Decoders (MOB - Mobile)

| Message Type (ID) | DCC Command (Trigger) | Other Triggers | Reference (RCN-217) |
| :--- | :--- | :--- | :--- |
| **ADR (1, 2)** | After every DCC packet addressed to any vehicle decoder. | - | Section 3.1, 5.2 |
| **Info1 (3)** | - | Cyclically with ADR (1, 2) if bit 3 in CV28 is enabled. | Section 5.2.2 |
| **POM (0)** | In response to a DCC "POM Read" command addressed to this decoder. | - | Section 5.1 |
| **EXT (3)** | In response to the DCC function command "XF1 off". | Used to transmit location information. | Section 5.3 |
| **DYN (7)** | - | Sent spontaneously by the decoder to report dynamic state changes (e.g., speed, fill level). | Section 5.5 |
| **XPOM (8-11)** | In response to a DCC "XPOM Read" command. | The sequence number (SS) in the response corresponds to that of the command. | Section 5.6 |
| **CV-Car (12)** | In response to the DCC function command "XF3 on" to transmit all CVs in the background. | Can also be sent spontaneously. | Section 5.7 |
| **Rerailing (1, 2, 14)** | In response to the DCC function command "XF2 off" (to broadcast address 0). | After the decoder has received power (e.g., after rerailing). | Section 5.2.3 |

## Accessory/Turnout Decoders (STAT - Stationary)

| Message Type (ID) | DCC Command (Trigger) | Other Triggers | Reference (RCN-217) |
| :--- | :--- | :--- | :--- |
| **SRQ (no ID)** | After any accessory decoder DCC packet or a NOP command. | When a service request is pending (e.g., due to manual operation). Sent on channel 1. | Section 6.1 |
| **POM (0)** | In response to a DCC "POM Read" command addressed to this decoder. | - | Section 6.2 |
| **STAT1 (4)** | As confirmation/response after a switching command for advanced accessory decoders. | - | Section 6.3 |
| **STAT4 (3)** | As confirmation/response after a switching command for simple accessory decoders. | Reports the state of 4 turnout pairs. | Section 6.4 |
| **TIME (5)** | As confirmation/response after a switching command. | To report the expected switching time. | Section 6.5 |
| **ERROR (6)** | After any command (including NOP). | When an error condition exists in the decoder. | Section 6.6 |
| **DYN (7)** | - | Sent spontaneously by the decoder to report dynamic state changes. | Section 6.7 |
| **XPOM (8-11)** | In response to a DCC "XPOM Read" command. | - | Section 6.8 |
| **STAT2 (8)** | As confirmation/response after a switching command. | Specifically for mechanical drives. | Section 6.9 |
