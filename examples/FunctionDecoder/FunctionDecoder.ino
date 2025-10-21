#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t DECODER_ADDRESS = 1234;

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0);
RailcomReceiver receiver(&stream);
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

    if (millis() - lastDccPacketTime > 3000) {
        lastDccPacketTime = millis();

        manager.sendDynamicData(5, fuelLevel);
        if (fuelLevel > 0) fuelLevel--;

        uint8_t dcc_data[] = { (uint8_t)(DECODER_ADDRESS >> 8), (uint8_t)DECODER_ADDRESS, 0 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}
