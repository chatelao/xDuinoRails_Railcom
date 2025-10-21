#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"

const uint16_t ACCESSORY_ADDRESS = 100;

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 0, 1);
RailcomManager manager(sender, receiver);

bool turnoutThrown = false;
unsigned long lastStateChange = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    sender.begin();
    receiver.begin();

    Serial.println("Accessory Decoder Example (Refactored)");
}

void loop() {
    sender.task();

    // Simulate receiving a turnout command every 5 seconds
    if (millis() - lastStateChange > 5000) {
        turnoutThrown = !turnoutThrown;
        lastStateChange = millis();

        Serial.print("Simulating DCC command: Turnout is now ");
        Serial.println(turnoutThrown ? "thrown" : "closed");

        // 1. Queue the status report
        uint8_t status = turnoutThrown ? 1 : 0;
        manager.sendStatus1(status);

        // 2. Send the DCC packet that triggers the cutout
        uint8_t dcc_data[] = { (uint8_t)(ACCESSORY_ADDRESS >> 8), (uint8_t)ACCESSORY_ADDRESS, 0 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
