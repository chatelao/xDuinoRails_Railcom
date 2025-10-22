#include <Arduino.h>
#include <NmraDcc.h>
#include "RailcomSender.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

RailcomSender sender(uart0, 0, 1);
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);
NmraDcc Dcc;

#define DCC_PIN 2

void notifyDccMsg(DCC_MSG* msg) {
    DCCMessage dcc_msg(msg->Data, msg->Size);
    stateMachine.handleDccPacket(dcc_msg);
    sender.send_dcc_with_cutout(dcc_msg);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Dcc.pin(DCC_PIN, 0);
    Dcc.init(MAN_ID_DIY, 10, FLAGS_MY_ADDRESS_ONLY, 0);
    sender.begin();

    Serial.println("Locomotive Decoder (NMRA) Example - State Machine Version");
}

void loop() {
    Dcc.process();
    sender.task();
}
