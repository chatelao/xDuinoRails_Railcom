#include <Arduino.h>
#include "RailcomTx.h"
#include "RailcomRx.h"

// Command Station on UART0
RailcomTx cs_railcomTx(uart0, 0, 1);
RailcomRx cs_railcomRx(uart0, 1);

// Locomotive Decoder on UART1
const uint16_t LOCO_ADDRESS = 1234;
const uint8_t CV_VALUE = 42;
RailcomTx loco_railcomTx(uart1, 4, 5);
RailcomRx loco_railcomRx(uart1, 5);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_railcomTx.begin();
    cs_railcomRx.begin();
    loco_railcomTx.begin();
    loco_railcomRx.begin();

    Serial.println("In-Circuit Test (Refactored)");

    loco_railcomTx.sendPomResponse(CV_VALUE);

    DCCMessage dcc_msg;
    cs_railcomTx.send_dcc_with_cutout(dcc_msg);

    delay(1);
    loco_railcomTx.task();
    delay(10);

    RailcomMessage* msg = cs_railcomRx.readMessage();

    if (msg && msg->id == RailcomID::POM && static_cast<PomMessage*>(msg)->cvValue == CV_VALUE) {
        Serial.println("SUCCESS: In-circuit test passed!");
    } else {
        Serial.println("FAILURE: Did not receive correct POM response.");
    }
}

void loop() {
    cs_railcomTx.task();
    loco_railcomTx.task();
}
