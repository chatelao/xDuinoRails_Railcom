#include <Arduino.h>
#include "RailcomTx.h"
#include "RP2040RailcomHardware.h"
#include "DecoderStateMachine.h"

// Note: This sketch is a dummy and does not actually send DCC packets.
// It simulates the logic of an accessory decoder receiving DCC packets
// and using the DecoderStateMachine to generate RailCom responses.

const uint16_t ACCESSORY_ADDRESS = 100;
const uint8_t CV28 = 0b00000011; // Enable both channels
const uint8_t CV29 = 0b00001010; // Enable RailCom

// Hardware setup for UART
RP2040RailcomHardware hardware(uart0, 0, 1); // UART TX on GP0, UART RX on GP1
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
    // The main loop simulates receiving a DCC packet every 2 seconds.
    if (millis() - lastDccPacketTime > 2000) {
        lastDccPacketTime = millis();

        // --- Simulate a sequence of DCC accessory packets ---
        // This part constructs a fake DCC message.
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

        // 1. The state machine processes the packet and queues a RailCom response.
        stateMachine.handleDccPacket(dcc_msg);

        // 2. The main application would now create the RailCom cutout and then...
        // 3. Trigger the RailcomTx to send the queued message.
        railcomTx.on_cutout_start();
    }
}
