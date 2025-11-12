# Howto: RailCom Decoder (Transmitter)

This guide explains how to wire a Seeed Studio XIAO RP2040 and run a minimal software example to create a RailCom-enabled locomotive decoder. By the end, you will understand how to send Railcom messages from a decoder in response to DCC commands.

## Key Concepts

A RailCom transmitter (decoder) doesn't just send data; it has to do so in perfect synchronization with the DCC signal.

1.  **The "Cutout":** A RailCom decoder can only transmit during a brief, powerless gap in the DCC signal known as the "cutout."
2.  **Cutout Control Signal:** This library does **not** create the high-power cutout on the rails itself. Instead, it generates a precise **logic-level control signal** on a specified GPIO pin. This signal is intended to be connected to a DCC booster or command station that has a "cutout control" input, which then creates the actual cutout on the track.
3.  **Message Timing:** The library automatically sends its queued RailCom message via UART TX at the exact moment it asserts the cutout control signal.

## Wiring Diagram

This diagram shows a minimal setup for demonstration purposes. A real-world decoder would require a more robust circuit with opto-isolation and a full bridge rectifier to safely handle the DCC track signal.

**Components:**
- 1x Seeed Studio XIAO RP2040
- 1x 1kΩ resistor

**Connections:**
- The **DCC Signal** from the track is connected through a 1kΩ resistor to the XIAO's **UART TX pin (D6)**. This is how the RailCom message is physically sent.
- The **Track GND** provides a common ground reference.
- The **Cutout Control Signal** is generated on a separate GPIO pin (**D7**). This pin should be connected to the cutout control input of your DCC booster.

```ascii
                      RailCom Signal (to Track)
                            ▲
                            │
                      ┌─────┴─────┐
                      │ 1kΩ Resistor │
                      └─────┬─────┘
                            │
┌───────────────────────────┼───────────────────────────┐
│                           │                           │
│     ┌─────────────────────┴─────────────────────┐     │
│     │ D6/GPIO0 (UART0 TX)                       │     │
│     │                                           │     │
│     │             Seeed Studio XIAO             ├─────┤  Track GND
│     │                   RP2040                  │ GND │
│     │                                           │     │
│     │ D7/GPIO1 (PIO Cutout Control)             │     │
│     └─────────────────────┬─────────────────────┘     │
│                           │                           │
└───────────────────────────┼───────────────────────────┘
                            │
                            ▼
                To DCC Booster Cutout Control Input
```

## Minimal Software Example

This example demonstrates how to initialize the `RailcomTx` library and queue a RailCom message to be sent in response to a simulated DCC packet.

**Code:**
```cpp
#include <Arduino.h>
#include "RailcomTx.h"
#include "DecoderStateMachine.h"

// Define the locomotive address for this decoder.
const uint16_t LOCO_ADDRESS = 3;

// 1. Initialize the RailcomTx library.
//    - uart0: The hardware UART instance to use.
//    - TX Pin (0 -> D6 on XIAO): The GPIO pin for the UART TX signal.
//    - PIO Pin (1 -> D7 on XIAO): The GPIO pin that will generate the
//      logic-level signal to control the DCC cutout.
RailcomTx railcomTx(uart0, 0, 1);

// 2. Initialize the Decoder State Machine.
//    This helper class contains the logic to decide which RailCom message
//    to send in response to different DCC commands.
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

void setup() {
  // Start the RailCom transmitter. This configures the PIO and UART.
  railcomTx.begin();

  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for the serial port to connect.
  }
  Serial.println("RailCom TX Decoder Example Initialized");
}

void loop() {
  // 3. The railcomTx.task() function must be called repeatedly in the main loop.
  //    It handles the non-blocking sending of any queued messages.
  railcomTx.task();

  // --- In a real decoder, the following would be triggered by an NMRA DCC library ---

  // 4. Simulate receiving a DCC "speed and direction" packet for our address.
  DCCMessage dcc_msg;
  dcc_msg.len = 3;
  dcc_msg.data[0] = high(LOCO_ADDRESS); // Address High Byte
  dcc_msg.data[1] = low(LOCO_ADDRESS);  // Address Low Byte
  dcc_msg.data[2] = 0b01100000;         // Command (Speed/Direction)

  // 5. Pass the DCC packet to the state machine. It analyzes the packet
  //    and queues the appropriate RailCom response (e.g., an address broadcast).
  stateMachine.handleDccPacket(dcc_msg);

  // 6. Tell the library to generate the cutout. The RailcomTx library will now
  //    assert the PIO cutout control pin and simultaneously send the queued
  //    RailCom message on the UART TX pin.
  railcomTx.on_cutout_start();

  Serial.println("Simulated DCC packet received, RailCom response sent.");

  // Wait for a second before repeating the loop.
  delay(1000);
}
```

**Explanation:**

The workflow is straightforward:

1.  **Initialization:** In `setup()`, we create instances of `RailcomTx` and `DecoderStateMachine`. We tell `RailcomTx` which hardware pins to use for the UART signal and the PIO cutout control.
2.  **DCC Packet Arrival:** In a real decoder, a library like `NmraDcc` would notify you when a new DCC packet arrives. In our `loop()`, we simulate this by creating a sample `DCCMessage`.
3.  **Queueing a Response:** We pass this DCC message to our `stateMachine`. It contains the application logic and decides what message to send back. For a simple address broadcast, it will call `railcomTx.queueAddress()`.
4.  **Triggering the Send:** Finally, we call `railcomTx.on_cutout_start()`. This is the trigger. The library takes over, asserts the cutout control pin, and sends the queued message via the UART TX pin. The `railcomTx.task()` function, called at the top of the loop, ensures this process is handled correctly without blocking the rest of your code.
