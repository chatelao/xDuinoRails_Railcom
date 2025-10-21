#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0); // PIO pin is TX pin
RailcomReceiver receiver(&stream);
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
    sender.task();

    if (millis() - lastDccPacketTime > 1000) {
        lastDccPacketTime = millis();
        manager.sendAddress(LOCOMOTIVE_ADDRESS);

        uint8_t dcc_data[] = { (uint8_t)(LOCOMOTIVE_ADDRESS >> 8), (uint8_t)LOCOMOTIVE_ADDRESS, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
