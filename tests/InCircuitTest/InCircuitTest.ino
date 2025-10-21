#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

// Command Station on UART0
HardwareUartStream cs_stream(uart0, 0, 1);
RailcomSender cs_sender(&cs_stream, 0);
RailcomReceiver cs_receiver(&cs_stream);
RailcomManager cs_manager(cs_sender, cs_receiver);

// Locomotive Decoder on UART1
const uint16_t LOCO_ADDRESS = 1234;
const uint8_t CV_VALUE = 42;
HardwareUartStream loco_stream(uart1, 4, 5);
RailcomSender loco_sender(&loco_stream, 4);
RailcomReceiver loco_receiver(&loco_stream);
RailcomManager loco_manager(loco_sender, loco_receiver);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_sender.begin();
    cs_receiver.begin();
    loco_sender.begin();
    loco_receiver.begin();

    Serial.println("In-Circuit Test: POM Read (Refactored)");

    loco_manager.sendPomResponse(CV_VALUE);

    uint8_t dcc_data[] = { (uint8_t)(LOCO_ADDRESS >> 8), (uint8_t)LOCO_ADDRESS, 0};
    DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
    cs_sender.send_dcc_with_cutout(dcc_msg);

    delay(1);
    loco_sender.task();
    delay(10);

    RailcomMessage* msg = cs_manager.readMessage();

    if (msg != nullptr && msg->id == RailcomID::POM) {
        PomMessage* pom_msg = static_cast<PomMessage*>(msg);
        if (pom_msg->cvValue == CV_VALUE) {
            Serial.println("SUCCESS: In-circuit test passed!");
        } else {
            Serial.println("FAILURE: Incorrect CV value.");
        }
    } else {
        Serial.println("FAILURE: Did not receive a valid POM response.");
    }
}

void loop() {
    cs_sender.task();
    loco_sender.task();
}
