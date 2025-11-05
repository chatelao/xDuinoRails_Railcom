#include <Arduino.h>
#include "RailcomTx.h"
#include "DecoderStateMachine.h"
#include "RP2040RailcomHardware.h"

// --- Decoder Configuration ---
const uint16_t DECODER_ADDRESS = 4098;
const uint16_t MANUFACTURER_ID = 0x0123;
const uint32_t PRODUCT_ID = 0x456789AB;

RP2040RailcomHardware railcomHardware(uart0, 0, 2, 1);
RailcomTx railcomTx(&railcomHardware);
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, DECODER_ADDRESS, MANUFACTURER_ID, PRODUCT_ID);

void setup() {
    Serial.begin(115200);
    while(!Serial);
    railcomTx.begin();
    Serial.println("RCN-218 Logon Example");
    Serial.println("Decoder is in IDLE state, waiting for LOGON_ENABLE from Command Station...");
}

void loop() {
    railcomTx.task();
    // In a real application, you would parse incoming DCC messages from the track
    // and pass them to stateMachine.handleDccPacket().
    // For this example, we will manually simulate the DCC-A logon sequence.

    // 1. Simulate LOGON_ENABLE
    Serial.println("Simulating LOGON_ENABLE...");
    uint8_t logon_enable[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ENABLE | 1, 0x12, 0x34, 0x56 };
    DCCMessage logon_msg(logon_enable, sizeof(logon_enable));
    stateMachine.handleDccPacket(logon_msg);
    railcomTx.send_dcc_with_cutout(logon_msg);
    delay(2000);

    // 2. Simulate SELECT
    Serial.println("Simulating SELECT...");
    uint8_t select[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_SELECT | 1, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xFC };
    DCCMessage select_msg(select, sizeof(select));
    stateMachine.handleDccPacket(select_msg);
    railcomTx.send_dcc_with_cutout(select_msg);
    delay(2000);

    // 3. Simulate LOGON_ASSIGN
    Serial.println("Simulating LOGON_ASSIGN...");
    uint8_t logon_assign[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ASSIGN | 1, 0x23, 0x45, 0x67, 0x89, 0xAB, (DECODER_ADDRESS >> 8), (DECODER_ADDRESS & 0xFF) };
    DCCMessage assign_msg(logon_assign, sizeof(logon_assign));
    stateMachine.handleDccPacket(assign_msg);
    railcomTx.send_dcc_with_cutout(assign_msg);
    delay(2000);

    Serial.println("Logon sequence complete. Decoder is now registered.");
    while(1) {
        railcomTx.task();
        // Now the decoder would respond to normal DCC commands for its address
    }
}
