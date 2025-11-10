#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomTx.h"
#include "RP2040RailcomHardware.h"
#include "DecoderStateMachine.h"

#define DCC_PIN 2

NmraDcc Dcc;
// Correctly initialize the hardware with the new 3-argument constructor
RP2040RailcomHardware hardware(uart0, 1, 0); // UART TX on GP1, UART RX on GP0
RailcomTx railcomTx(&hardware);

// For this example, we'll let the NmraDcc library manage the address.
// We'll use standard default values for the CVs.
const uint8_t CV28 = 0b00000011; // Enable both channels
DecoderStateMachine stateMachine(railcomTx, DecoderType::ACCESSORY, 0, CV28, CV29_ACCESSORY_DECODER);


void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
  // We can't get the raw packet data from the new API, so we have to reconstruct it
  // for the state machine to process.
  uint8_t addr_high = 0b10000000 | ((Addr - 1) >> 2);
  uint8_t addr_low = ((Addr - 1) & 0b11) << 4;
  uint8_t cmd_byte = 0b10001000 | addr_low | (Direction ? 1 : 0) | (OutputPower ? 0x80 : 0);
  uint8_t data[] = {addr_high, cmd_byte, (uint8_t)(addr_high ^ cmd_byte)};
  DCCMessage dcc_msg(data, sizeof(data));
  stateMachine.handleDccPacket(dcc_msg);

  // After the state machine has queued a response, trigger the transmission.
  railcomTx.on_cutout_start();
}

void setup() {
  Serial.begin(115200);
  railcomTx.begin();

  Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, false);
  Dcc.init(MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER, 0);
}

void loop() {
  Dcc.process();
  // railcomTx.task() has been removed in the new API.
  // The sending is now triggered directly in the DCC callback.
}
