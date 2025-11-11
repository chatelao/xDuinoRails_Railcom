#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomTx.h"
#include "RP2040RailcomTxHardware.h"
#include "DecoderStateMachine.h"

#define DCC_PIN 2

// --- Configuration Variables ---
// These would typically be stored in EEPROM or flash in a real decoder.

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

// CV28: RailCom Configuration
const uint8_t CV28 = 0b00000011; // Enable both channels

// CV29: Decoder Configuration
const uint8_t CV29 = 0b00001010; // Enable RailCom, 28/128 speed steps, long address

// Hardware setup for UART
RP2040RailcomTxHardware hardware(uart0, 0); // UART TX on GP0
RailcomTx railcomTx(&hardware);
// Initialize the state machine with the decoder's address and CV configuration.
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS, CV28, CV29);
NmraDcc Dcc;

void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps) {
  // This sketch demonstrates how to integrate with the NmraDcc library.
  // When a DCC packet is received, the state machine is updated,
  // which in turn queues a RailCom message. Then, we trigger the send.

  // We can't get the raw packet data from the new API, so we have to reconstruct it
  // for the state machine. This is a simplified reconstruction.
  if (AddrType == DCC_ADDR_LONG) {
    uint8_t addr_high = 0b11000000 | (Addr >> 8);
    uint8_t addr_low = Addr & 0xFF;
    uint8_t cmd_byte = 0b01100000 | (Speed & 0x1F); // Simple speed for now
    uint8_t data[] = {addr_high, addr_low, cmd_byte, (uint8_t)(addr_high ^ addr_low ^ cmd_byte)};
    DCCMessage dcc_msg(data, sizeof(data));
    stateMachine.handleDccPacket(dcc_msg);
  } else {
    uint8_t addr_byte = Addr;
    uint8_t cmd_byte = 0b01100000 | (Speed & 0x1F);
    uint8_t data[] = {addr_byte, cmd_byte, (uint8_t)(addr_byte ^ cmd_byte)};
    DCCMessage dcc_msg(data, sizeof(data));
    stateMachine.handleDccPacket(dcc_msg);
  }

  // After the DCC packet is processed by the state machine, a RailCom
  // message might be queued. The application is now responsible for creating
  // the cutout and calling on_cutout_start() to send the RailCom data.
  // For this example, we'll just call it directly.
  railcomTx.on_cutout_start();
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
}
