#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomTx.h"
#include "RP2040RailcomHardware.h"
#include "DecoderStateMachine.h"

#define DCC_PIN 2

// --- Configuration Variables ---
// These would typically be stored in EEPROM or flash in a real decoder.

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

// CV28: RailCom Configuration
// Bit 0: Channel 1 Enable (1=Enabled)
// Bit 1: Channel 2 Enable (1=Enabled)
// Note: This library currently uses Channel 1 for address broadcast and
// Channel 2 for all other responses.
const uint8_t CV28 = 0b00000011; // Enable both channels

// CV29: Decoder Configuration
// Bit 3: RailCom Enable (1=Enabled)
// For a full list of bits, see NMRA S-9.2.2.
// To disable all RailCom transmissions from this decoder, set this value to
// e.g., 0b00000010 (disable bit 3).
const uint8_t CV29 = 0b00001010; // Enable RailCom, 28/128 speed steps, long address

// RX pin 3 is a placeholder, as this example does not receive DCC.
RP2040RailcomHardware hardware(uart0, 0, 3);
RailcomTx railcomTx(&hardware);
// Initialize the state machine with the decoder's address and CV configuration.
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS, CV28, CV29);
NmraDcc Dcc;

void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps) {
  // We can't get the raw packet data from the new API, so we have to reconstruct it.
  // This is a simplified reconstruction and may not be 100% correct for all cases.
  if (AddrType == DCC_ADDR_LONG) {
    uint8_t addr_high = 0b11000000 | (Addr >> 8);
    uint8_t addr_low = Addr & 0xFF;
    uint8_t cmd_byte = 0b01100000 | (Speed & 0x1F); // Simple speed for now
    uint8_t data[] = {addr_high, addr_low, cmd_byte, (uint8_t)(addr_high ^ addr_low ^ cmd_byte)};
    DCCMessage dcc_msg(data, sizeof(data));
    stateMachine.handleDccPacket(dcc_msg);
    // In a real application, you would detect the RailCom cutout here and then call:
    railcomTx.on_cutout_start();
  } else {
    uint8_t addr_byte = Addr;
    uint8_t cmd_byte = 0b01100000 | (Speed & 0x1F);
    uint8_t data[] = {addr_byte, cmd_byte, (uint8_t)(addr_byte ^ cmd_byte)};
    DCCMessage dcc_msg(data, sizeof(data));
    stateMachine.handleDccPacket(dcc_msg);
    // In a real application, you would detect the RailCom cutout here and then call:
    railcomTx.on_cutout_start();
  }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, false);
    Dcc.init(MAN_ID_DIY, 10, 0, 0);
    railcomTx.begin();

    Serial.println("Locomotive Decoder (NMRA) Example - State Machine Version");
}

void loop() {
    Dcc.process();
    railcomTx.task();
}
