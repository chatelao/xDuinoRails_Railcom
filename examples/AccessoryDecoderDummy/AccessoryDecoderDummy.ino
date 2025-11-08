#include <Arduino.h>
#include "RailcomTx.h"
#include "RP2040RailcomHardware.h"
#include "DecoderStateMachine.h"

const uint16_t ACCESSORY_ADDRESS = 100;
const uint8_t CV28 = 0b00000011; // Enable both channels
const uint8_t CV29 = 0b00001010; // Enable RailCom

RP2040RailcomHardware hardware(uart0, 0, 1, 2); // RX pin 2 is a placeholder
RailcomTx railcomTx(&hardware);
DecoderStateMachine stateMachine(railcomTx, DecoderType::ACCESSORY, ACCESSORY_ADDRESS, CV28, CV29);

unsigned long lastDccPacketTime = 0;
int command_step = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);
    railcomTx.begin();
    Serial.println("Accessory Decoder (Dummy) Example - State Machine v2");
}

void loop() {
    railcomTx.task();

    if (millis() - lastDccPacketTime > 2000) {
        lastDccPacketTime = millis();

        // --- Simulate a sequence of DCC accessory packets ---
        uint8_t dcc_data[2];
        uint16_t addr_part = ACCESSORY_ADDRESS - 1;
        dcc_data[0] = 0b10000000 | (~(addr_part >> 2));
        dcc_data[1] = 0b10000000 | ((addr_part & 0x03) << 1);

        switch (command_step) {
            case 0: // Output 0 ON
                dcc_data[1] |= 0b00001000;
                Serial.println("Simulating DCC: Address 100, Output 0 ON");
                break;
            case 1: // Output 1 ON
                dcc_data[1] |= 0b00001001;
                Serial.println("Simulating DCC: Address 100, Output 1 ON");
                break;
            case 2: // Output 0 OFF
                dcc_data[1] |= 0b00000000;
                Serial.println("Simulating DCC: Address 100, Output 0 OFF");
                break;
            case 3: // Output 1 OFF
                dcc_data[1] |= 0b00000001;
                Serial.println("Simulating DCC: Address 100, Output 1 OFF");
                break;
        }
        command_step = (command_step + 1) % 4;

        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        stateMachine.handleDccPacket(dcc_msg);
        railcomTx.send_dcc_with_cutout(dcc_msg);
    }
}
