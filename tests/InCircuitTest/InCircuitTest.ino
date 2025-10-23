#include <Arduino.h>
#include "RailcomTx.h"
#include "RailcomRx.h"

// Command Station on UART0
RailcomTx cs_tx(uart0, 0, 1);
RailcomRx cs_rx(uart0, 1);

// Locomotive Decoder on UART1
const uint16_t LOCO_ADDRESS = 1234;
const uint8_t CV_VALUE = 42;
RailcomTx loco_tx(uart1, 4, 5);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_tx.begin();
    cs_rx.begin();
    loco_tx.begin();

    Serial.println("In-Circuit Test (Refactored)");

    loco_tx.sendPomResponse(CV_VALUE);

    DCCMessage dcc_msg;
    cs_tx.send_dcc_with_cutout(dcc_msg);

    delay(5);
    loco_tx.task();
    delay(10);

    RailcomMessage* msg = cs_rx.readMessage();

    if (msg && msg->id == RailcomID::POM && static_cast<PomMessage*>(msg)->cvValue == CV_VALUE) {
        Serial.println("SUCCESS: In-circuit test passed!");
    } else {
        Serial.println("FAILURE: Did not receive correct POM response.");
    }
}

void loop() {
    cs_tx.task();
    loco_tx.task();
}
