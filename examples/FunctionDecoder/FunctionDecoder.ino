#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"

const uint16_t DECODER_ADDRESS = 1234;

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 0, 1);
RailcomManager manager(sender, receiver);

unsigned long lastDccPacketTime = 0;
uint8_t fuelLevel = 100;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    sender.begin();
    receiver.begin();

    Serial.println("Function Decoder Example (Refactored)");
}

void loop() {
    sender.task();

    // Simulate receiving a DCC packet every 3 seconds
    if (millis() - lastDccPacketTime > 3000) {
        lastDccPacketTime = millis();

        // 1. Queue the DYN message with the current fuel level
        Serial.print("Queuing DYN message: Fuel Level = ");
        Serial.println(fuelLevel);
        manager.sendDynamicData(5, fuelLevel); // Subindex 5 for "container 1"
        if (fuelLevel > 0) fuelLevel--;

        // 2. Send the DCC packet that triggers the cutout
        uint8_t dcc_data[] = { (uint8_t)(DECODER_ADDRESS >> 8), (uint8_t)DECODER_ADDRESS, 0 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
