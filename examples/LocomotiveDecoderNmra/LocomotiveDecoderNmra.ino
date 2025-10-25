#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomTx.h"
#include "DecoderStateMachine.h"

#define DCC_PIN 2
const uint16_t LOCOMOTIVE_ADDRESS = 4098;

RailcomTx railcomTx(uart0, 0, 1);
DecoderStateMachine stateMachine(railcomTx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);
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
    railcomTx.send_dcc_with_cutout(dcc_msg);
  } else {
    uint8_t addr_byte = Addr;
    uint8_t cmd_byte = 0b01100000 | (Speed & 0x1F);
    uint8_t data[] = {addr_byte, cmd_byte, (uint8_t)(addr_byte ^ cmd_byte)};
    DCCMessage dcc_msg(data, sizeof(data));
    stateMachine.handleDccPacket(dcc_msg);
    railcomTx.send_dcc_with_cutout(dcc_msg);
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
