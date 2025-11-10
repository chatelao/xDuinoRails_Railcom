#include <Arduino.h>
#include "Railcom.h"
#include "RailcomTx.h"
#include "RP2040RailcomHardware.h"
#include "DecoderStateMachine.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

// For RCN-218 automatic logon, you would also provide the manufacturer and product IDs:
// const uint16_t MANUFACTURER_ID = 0x0123;
// const uint32_t PRODUCT_ID = 0x456789AB;
// DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS, CV28, CV29, MANUFACTURER_ID, PRODUCT_ID);

const uint8_t CV28 = 0b00000011; // Enable both channels
const uint8_t CV29 = 0b00001010; // Enable RailCom

// RX pin 2 is a placeholder, as this example does not receive DCC.
RP2040RailcomHardware hardware(uart0, 0, 2);
RailcomTx railcomTx(&hardware);
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS, CV28, CV29);

unsigned long lastDccPacketTime = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    railcomTx.begin();
    Serial.println("Locomotive Decoder (Dummy) Example - State Machine Version");
}

void loop() {
    railcomTx.task();

    // Simulate receiving a DCC packet addressed to us every second
    if (millis() - lastDccPacketTime > 1000) {
        lastDccPacketTime = millis();

        // 1. Create a simulated DCC packet
        uint8_t dcc_data[] = { (uint8_t)(LOCOMOTIVE_ADDRESS >> 8), (uint8_t)LOCOMOTIVE_ADDRESS, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));

        // 2. Let the state machine decide what to queue
        stateMachine.handleDccPacket(dcc_msg);

        // 3. In a real decoder, the cutout would be detected after sending the DCC packet.
        //    Here, we simulate that detection by calling on_cutout_start() directly.
        railcomTx.on_cutout_start();
    }
}
