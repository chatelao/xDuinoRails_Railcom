#include <Arduino.h>
#include "RailcomTx.h"
#include "RP2040RailcomTxHardware.h"
#include "DecoderStateMachine.h"

// Note: This sketch is a dummy and does not actually send DCC packets.

const uint16_t DECODER_ADDRESS = 1234;
const uint8_t CV28 = 0b00000011; // Enable both channels
const uint8_t CV29 = 0b00001010; // Enable RailCom

// Hardware setup for UART
RP2040RailcomTxHardware hardware(uart0, 0); // UART TX on GP0
RailcomTx railcomTx(&hardware);
// Note: We'll re-use the LOCOMOTIVE type for the function decoder
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, DECODER_ADDRESS, CV28, CV29);

unsigned long lastDccPacketTime = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    railcomTx.begin();
    Serial.println("Function Decoder (Dummy) Example");
}

void loop() {
    if (millis() - lastDccPacketTime > 3000) {
        lastDccPacketTime = millis();

        // Simulate a simplified function packet
        uint8_t dcc_data[] = { (uint8_t)(DECODER_ADDRESS >> 8), (uint8_t)DECODER_ADDRESS, 0b10000000 };
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));

        // Let the state machine decide what to queue (it will queue an ADR broadcast)
        stateMachine.handleDccPacket(dcc_msg);

        // Trigger the RailcomTx to send the queued message
        railcomTx.on_cutout_start();
    }
}
