#include <Arduino.h>
#include "RailcomSender.h"
#include "RailcomReceiver.h"
#include "RailcomTxManager.h"
#include "RailcomRxManager.h"

// Command Station on UART0
RailcomSender cs_sender(uart0, 0, 1);
RailcomReceiver cs_receiver(uart0, 1);
RailcomTxManager cs_tx_manager(cs_sender);
RailcomRxManager cs_rx_manager(cs_receiver);

// Locomotive Decoder on UART1
const uint16_t LOCO_ADDRESS = 1234;
const uint8_t CV_VALUE = 42;
RailcomSender loco_sender(uart1, 4, 5);
RailcomReceiver loco_receiver(uart1, 5);
RailcomTxManager loco_tx_manager(loco_sender);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_sender.begin();
    cs_receiver.begin();
    loco_sender.begin();
    loco_receiver.begin();

    Serial.println("In-Circuit Test (Refactored)");

    loco_tx_manager.sendPomResponse(CV_VALUE);

    DCCMessage dcc_msg;
    cs_sender.send_dcc_with_cutout(dcc_msg);

    delay(1);
    loco_sender.task();
    delay(10);

    RailcomMessage* msg = cs_rx_manager.readMessage();

    if (msg && msg->id == RailcomID::POM && static_cast<PomMessage*>(msg)->cvValue == CV_VALUE) {
        Serial.println("SUCCESS: In-circuit test passed!");
    } else {
        Serial.println("FAILURE: Did not receive correct POM response.");
    }
}

void loop() {
    cs_sender.task();
    loco_sender.task();
}
