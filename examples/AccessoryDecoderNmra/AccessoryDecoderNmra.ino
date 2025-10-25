#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomTx.h"
#include "DecoderStateMachine.h"

#define DCC_PIN 2

NmraDcc Dcc;
RailcomTx railcomTx(uart0, 1, 0);
DecoderStateMachine stateMachine(railcomTx, DecoderType::ACCESSORY, 0);

void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower) {
  // We can't get the raw packet data from the new API, so we have to reconstruct it.
  // This is a simplified reconstruction and may not be 100% correct for all cases.
  uint8_t addr_high = 0b10000000 | ((Addr - 1) >> 2);
  uint8_t addr_low = ((Addr - 1) & 0b11) << 4;
  uint8_t cmd_byte = 0b10001000 | addr_low | (Direction ? 1 : 0) | (OutputPower ? 0x80 : 0);
  uint8_t data[] = {addr_high, cmd_byte, (uint8_t)(addr_high ^ cmd_byte)};
  DCCMessage dcc_msg(data, sizeof(data));
  stateMachine.handleDccPacket(dcc_msg);
}

void setup() {
  Serial.begin(115200);
  railcomTx.begin();

  Dcc.pin(digitalPinToInterrupt(DCC_PIN), DCC_PIN, false);
  Dcc.init(MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER, 0);
}

void loop() {
  Dcc.process();
  railcomTx.task();
}
