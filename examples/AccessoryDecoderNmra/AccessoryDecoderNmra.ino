#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomTx.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t ACCESSORY_ADDRESS = 100;

RailcomTx sender(uart0, 0, 1);
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::ACCESSORY, ACCESSORY_ADDRESS);
NmraDcc dcc;

void notifyDccAccPacket(uint16_t address, bool activate, uint8_t output, bool C, bool S) {
    DCCMessage dcc_msg(dcc.getPacket(), dcc.getPacketSize());
    stateMachine.handleDccPacket(dcc_msg);
    sender.send_dcc_with_cutout(dcc_msg);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    dcc.begin(INPUT_PIN, 0);
    dcc.setAccPacketHandler(notifyDccAccPacket, true);
    sender.begin();

    Serial.println("Accessory Decoder (NMRA) Example - State Machine Version");
}

void loop() {
    dcc.process();
    sender.task();
}
