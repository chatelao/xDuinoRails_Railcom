#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 0, 1);
RailcomManager manager(sender, receiver);

unsigned long lastDccPacketTime = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    sender.begin();
    receiver.begin();

    Serial.println("Locomotive Decoder Example (Refactored)");
}

void loop() {
    // The sender's task method must be called continuously to handle message sending
    // after the ISR has fired.
    sender.task();

    // A real decoder would receive DCC packets continuously.
    // We simulate receiving a packet addressed to us every second.
    if (millis() - lastDccPacketTime > 1000) {
        lastDccPacketTime = millis();

        // 1. Queue the RailCom message (address broadcast)
        manager.sendAddress(LOCOMOTIVE_ADDRESS);

        // 2. Send a DCC packet to create the cutout
        Serial.println("Simulating DCC packet, then sending queued RailCom message...");
        uint8_t dcc_data[] = { (uint8_t)(LOCOMOTIVE_ADDRESS >> 8), (uint8_t)LOCOMOTIVE_ADDRESS, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
