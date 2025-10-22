#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomTx.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

RailcomTx sender(uart0, 0, 1);
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);
NmraDcc dcc;

void notifyDccSpeedPacket(uint16_t address, DCC_ADDR_TYPE addr_type, uint8_t speed, DCC_DIRECTION forward) {
    // We can get the raw packet from the NmraDcc library
    DCCMessage dcc_msg(dcc.getPacket(), dcc.getPacketSize());

    // Let the state machine handle the logic
    stateMachine.handleDccPacket(dcc_msg);

    // Trigger the cutout so the message can be sent
    sender.send_dcc_with_cutout(dcc_msg);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    dcc.begin(INPUT_PIN, 0);
    dcc.setSpeedPacketHandler(notifyDccSpeedPacket, true);
    sender.begin();

    Serial.println("Locomotive Decoder (NMRA) Example - State Machine Version");
}

void loop() {
    dcc.process();
    sender.task();
}
