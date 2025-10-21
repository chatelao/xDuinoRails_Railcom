#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t ACCESSORY_ADDRESS = 100;

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0);
RailcomReceiver receiver(&stream);
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

    if (millis() - lastStateChange > 5000) {
        turnoutThrown = !turnoutThrown;
        lastStateChange = millis();

        Serial.print("Simulating DCC command: Turnout is now ");
        Serial.println(turnoutThrown ? "thrown" : "closed");

        uint8_t status = turnoutThrown ? 1 : 0;
        manager.sendStatus1(status);

        uint8_t dcc_data[] = { (uint8_t)(ACCESSORY_ADDRESS >> 8), (uint8_t)ACCESSORY_ADDRESS, 0 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
