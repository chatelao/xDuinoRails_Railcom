# Howto: RailCom Decoder (Transmitter)

This guide explains how to wire a Seeed Xiao RP2040 and run a minimal software example to create a RailCom-enabled locomotive decoder.

## How It Works

A RailCom-enabled decoder (like the one in this example) does not generate the RailCom cutout itself. Instead, it relies on the DCC command station to create the cutout on the rails immediately after a DCC packet is sent.

The decoder's firmware must:
1.  Receive a DCC packet.
2.  Process the packet and queue an appropriate RailCom response using this library.
3.  Detect the cutout period on the track that follows the DCC packet.
4.  Call the `on_cutout_start()` function of this library at the beginning of the detected cutout.

This library then handles the low-level details of sending the queued message with the correct timing and baud rate.

## Wiring

The wiring for a simple transmitter is straightforward.

**Components:**
- 1x Seeed Xiao RP2040
- 1x 1kΩ resistor

**Connections:**
1.  Connect the DCC signal from your command station to one of the track rails.
2.  Connect the other track rail to the GND pin on the Seeed Xiao RP2040.
3.  Connect the DCC signal rail through a 1kΩ resistor to the TX pin of the Seeed Xiao RP2040's UART0 (Pin D6/GPIO0). This is where the RailCom signal is sent.

**Diagram:**

```
               ┌───────────────┐
DCC Signal ----│ 1kΩ Resistor  ├---- (TX) D6/GPIO0 on Xiao RP2040
               └───────────────┘

Track GND -------------------------- GND on Xiao RP2040
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

// Initialize the RailcomTx library on UART0, using D6 as the TX pin.
RailcomTx railcomTx(uart0, 0); // UART instance, TX Pin (GPIO0 -> D6)

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

  // In a real decoder, you would send the DCC message to the motor driver here.
  // This library is not involved in that step.

  // Immediately after the DCC message is sent, the command station creates a
  // "cutout" on the rails. Your firmware must detect this cutout.
  // Once detected, you must call on_cutout_start() to send the RailCom message.
  railcomTx.on_cutout_start();

  Serial.println("Sent DCC packet and RailCom message.");

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
