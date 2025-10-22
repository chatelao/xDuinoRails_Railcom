#include <Arduino.h>
#include "RailcomTx.h"
#include "DecoderStateMachine.h"

const uint16_t DECODER_ADDRESS = 1234;

RailcomTx railcomTx(uart0, 0, 1);
// Note: We'll re-use the LOCOMOTIVE type for the function decoder
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, DECODER_ADDRESS);

unsigned long lastDccPacketTime = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    railcomTx.begin();
    Serial.println("Function Decoder (Dummy) Example");
}

void loop() {
    railcomTx.task();

    if (millis() - lastDccPacketTime > 3000) {
        lastDccPacketTime = millis();

        // This is a simplified function packet
        uint8_t dcc_data[] = { (uint8_t)(DECODER_ADDRESS >> 8), (uint8_t)DECODER_ADDRESS, 0b10000000 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));

        // Let the state machine decide what to queue (it will queue an ADR broadcast)
        stateMachine.handleDccPacket(dcc_msg);

        railcomTx.send_dcc_with_cutout(dcc_msg);
    }
}
