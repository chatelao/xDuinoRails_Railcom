# How-To: Build a Command Station Receiver

This guide explains how to use the `CommandStationRx` class to easily receive and react to RailCom messages from decoders on your layout. This class is ideal for building block occupancy detectors, automated control logic, or tools for reading CVs on the main.

## The Problem: Raw RailCom is Complex

The raw RailCom protocol involves many different message types. A simple action like identifying a locomotive with a long address requires you to:
1.  Receive an `ADR_HIGH` message.
2.  Store the high part of the address.
3.  Wait for the corresponding `ADR_LOW` message.
4.  Combine the two parts to get the full address.

This requires you to manage state and parse different message types manually.

## The Solution: `CommandStationRx`

The `CommandStationRx` class is a high-level handler that does all this work for you. It wraps the low-level `RailcomRx` and provides a simple, event-based system using callbacks. Instead of parsing messages, you just tell the handler what to do when a specific event happens.

### Core Concepts

1.  **Callbacks**: You write a function (a "callback") to handle a specific event.
2.  **Attachment**: You "attach" your function to an event using one of the `on...()` methods.
3.  **Task Processing**: You call the `task()` method in your main `loop()`. The `CommandStationRx` class will listen for RailCom messages and automatically run your callback function with the fully parsed data when the event occurs.

## Example 1: Simple Block Occupancy Detector

Let's build a detector that prints the address of any locomotive that enters a section of track. This is the "Hello, World!" of RailCom receiving.

### 1. Include the Libraries
```cpp
#include "RP2040RailcomRxHardware.h"
#include "RailcomRx.h"
#include "CommandStationRx.h"
```

### 2. Create the Global Objects
```cpp
RP2040RailcomRxHardware railcom_hw(uart1, 5); // Use UART1, RX on pin GP5
RailcomRx railcom_rx(&railcom_hw);
CommandStationRx command_station(&railcom_rx);
```

### 3. Write Your Callback Function
This function will be executed every time a full address is successfully received and assembled.
```cpp
void handleLocoIdentified(uint16_t address) {
  Serial.print("Block Occupancy: Locomotive with address ");
  Serial.print(address);
  Serial.println(" detected.");
}
```

### 4. Put it all Together in `setup()` and `loop()`
```cpp
void setup() {
  Serial.begin(115200);

  // Initialize the hardware and receiver
  railcom_rx.begin();

  // Initialize the command station handler
  command_station.begin();

  // Attach our function to the "loco identified" event
  command_station.onLocoIdentified(handleLocoIdentified);

  Serial.println("Detector ready. Waiting for locomotives...");
}

void loop() {
  // This one line does all the magic!
  command_station.task();
}
```
That's it! Now, whenever a locomotive sends its address, the `handleLocoIdentified` function will be called automatically, and the address will be printed.

## Available Events

You can attach callbacks for various events:

- **`onLocoIdentified(callback)`**: Triggered when a full locomotive address is received.
  - *Callback Signature*: `void(uint16_t address)`

- **`onPomResponse(callback)`**: Triggered when a decoder sends a POM (Program on Main) response.
  - *Callback Signature*: `void(uint16_t sourceAddress, uint8_t cvValue)`

- **`onAccessoryStatus(callback)`**: Triggered when an accessory decoder (e.g., for a turnout) reports its status.
  - *Callback Signature*: `void(uint16_t accessoryAddress, uint8_t status)`

By using the `CommandStationRx` class, you can build powerful, event-driven applications without needing to worry about the low-level details of the RailCom protocol.
