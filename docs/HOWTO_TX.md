# Howto: RailCom Decoder (Transmitter)

This guide explains how to wire a Seeed Xiao RP2040 and run a minimal software example to create a RailCom-enabled locomotive decoder.

## The Role of the PIO Pin

A common point of confusion is the role of the PIO pin. The RailCom specification requires a "cutout" in the DCC signal, a brief period where the track is unpowered, allowing the decoder to transmit its message.

- The **`RailcomTx` library does not create the physical cutout on the high-power DCC rails itself.**
- Instead, it generates a precise **logic-level control signal** on the specified PIO pin (D7 in the example).
- This control signal is intended to be connected to a DCC booster or command station that is capable of creating the actual cutout on the track.
- The library then sends the RailCom message on the UART TX pin during the time it asserts the cutout signal.

## Wiring

The following wiring is required to connect the Seeed Xiao RP2040 to your DCC track to send RailCom messages.

**Components:**
- 1x Seeed Xiao RP2040
- 1x 1kΩ resistor

**Connections:**
1.  Connect the DCC signal from your command station to one of the track rails.
2.  Connect the other track rail to the GND pin on the Seeed Xiao RP2040.
3.  Connect the DCC signal rail through a 1kΩ resistor to the TX pin of the Seeed Xiao RP2040's UART0 (Pin D6/GPIO0). This is the RailCom message transmission pin.
4.  Connect the PIO pin (Pin D7/GPIO1) to the cutout control input of your DCC booster. This pin outputs the cutout control signal.

**Diagram:**

```
                                  ┌────────────────────┐
DCC Booster Cutout Control Input ─┤ D7/GPIO1 (PIO Pin) │
                                  │                    │
               ┌───────────────┐  │                    │
DCC Signal ----│ 1kΩ Resistor  ├--┤ D6/GPIO0 (TX Pin)  │
               └───────────────┘  │   Seeed Xiao       │
                                  │      RP2040        │
Track GND ------------------------┤ GND                │
                                  └────────────────────┘
```

**Note:** This is a very basic circuit for demonstration purposes. A real-world decoder would have a more robust circuit including opto-isolation and a full bridge rectifier.

## Minimal Software Example

This example demonstrates how to initialize the `RailcomTx` library and send a RailCom message in response to a simulated DCC packet.

**Code:**

```cpp
#include <Arduino.h>
#include "RailcomTx.h"
#include "DecoderStateMachine.h"

// Define the locomotive address
const uint16_t LOCO_ADDRESS = 3;

// Initialize the RailcomTx library.
// - uart0: The UART instance to use for sending RailCom messages.
// - TX Pin (GPIO0 -> D6): The pin connected to the DCC rail for transmitting the RailCom signal.
// - PIO Pin (GPIO1 -> D7): The pin that will output a logic-level signal to control the DCC cutout.
//   This signal should be connected to a DCC booster that can create the physical cutout.
RailcomTx railcomTx(uart0, 0, 1);

// Initialize the Decoder State Machine
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

void setup() {
  // Start the RailCom transmitter
  railcomTx.begin();

  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("RailCom TX Example Initialized");
}

void loop() {
  // The railcomTx.task() function must be called repeatedly in the main loop
  // to handle the non-blocking sending of queued messages.
  railcomTx.task();

  // --- Simulate receiving a DCC packet for our address ---
  // In a real decoder, this would come from an NMRA DCC library.
  DCCMessage dcc_msg;
  dcc_msg.len = 3;
  dcc_msg.data[0] = high(LOCO_ADDRESS);
  dcc_msg.data[1] = low(LOCO_ADDRESS);
  dcc_msg.data[2] = 0b01100000; // Speed and direction

  // The DecoderStateMachine decides what message to send back.
  stateMachine.handleDccPacket(dcc_msg);

  // This function sends the DCC message and creates the cutout for the RailCom message.
  // The queued RailCom message from the state machine will be sent during the cutout.
  railcomTx.send_dcc_with_cutout(dcc_msg);

  Serial.println("Sent DCC packet with RailCom cutout.");

  // Wait before sending the next packet
  delay(1000);
}
```

**Explanation:**

1.  **`#include` directives:** We include the necessary library headers.
2.  **`LOCO_ADDRESS`:**  The DCC address of our simulated locomotive.
3.  **`RailcomTx railcomTx(...)`:** We create an instance of the `RailcomTx` class, telling it to use `uart0`, pin `0` (which is D6 on the Xiao RP2040) for transmitting, and pin `1` (D7) for the PIO cutout signal.
4.  **`DecoderStateMachine stateMachine(...)`:** This helper class encapsulates the logic for which RailCom message to send.
5.  **`setup()`:** We initialize the `RailcomTx` library.
6.  **`loop()`:**
    *   `railcomTx.task()`: This is essential for the non-blocking operation of the library.
    *   We create a `DCCMessage` to simulate a packet being received.
    *   `stateMachine.handleDccPacket(dcc_msg)`: We pass the DCC packet to the state machine, which queues the appropriate RailCom response.
    *   `railcomTx.send_dcc_with_cutout(dcc_msg)`: This function is the key. It simulates sending the DCC packet to the motor and, critically, uses the PIO to create the RailCom cutout in the DCC signal. The message queued by the state machine is automatically sent during this cutout.
