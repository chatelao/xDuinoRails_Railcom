#include <Arduino.h>
#include "RailcomSender.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t ACCESSORY_ADDRESS = 100;

RailcomSender sender(uart0, 0, 1);
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::ACCESSORY, ACCESSORY_ADDRESS);

unsigned long lastDccPacketTime = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    sender.begin();
    Serial.println("Accessory Decoder (Dummy) Example - State Machine Version");
}

void loop() {
    sender.task();

    if (millis() - lastDccPacketTime > 5000) {
        lastDccPacketTime = millis();

        uint8_t dcc_data[] = { (uint8_t)(ACCESSORY_ADDRESS >> 8), (uint8_t)ACCESSORY_ADDRESS, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));

        stateMachine.handleDccPacket(dcc_msg);

        sender.send_dcc_with_cutout(dcc_msg);
    }
}
