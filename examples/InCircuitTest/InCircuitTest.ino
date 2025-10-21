#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"

// Command Station on UART0
RailcomSender cs_sender(uart0, 0, 1);
RailcomReceiver cs_receiver(uart0, 0, 1);
RailcomManager cs_manager(cs_sender, cs_receiver);

// Locomotive Decoder on UART1
const uint16_t LOCO_ADDRESS = 1234;
const uint8_t CV_VALUE = 42;
RailcomSender loco_sender(uart1, 4, 5);
RailcomReceiver loco_receiver(uart1, 4, 5);
RailcomManager loco_manager(loco_sender, loco_receiver);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_sender.begin();
    cs_receiver.begin();
    loco_sender.begin();
    loco_receiver.begin();

    Serial.println("In-Circuit Test: POM Read (Refactored)");

    // --- Loco: Queue a POM response ---
    // This simulates the loco getting a request and preparing a response.
    loco_manager.sendPomResponse(CV_VALUE);

    // --- CS: Send DCC packet to trigger cutout ---
    Serial.println("Command Station: Sending DCC packet to trigger cutout...");
    uint8_t dcc_data[] = { (uint8_t)(LOCO_ADDRESS >> 8), (uint8_t)LOCO_ADDRESS, 0};
    DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
    cs_sender.send_dcc_with_cutout(dcc_msg);

    // We need to run the task method for the sender to actually send the data
    // after the ISR fires.
    delay(1); // Give time for ISR to fire
    loco_sender.task();
    delay(10);

    // --- CS: Read and verify the response ---
    Serial.println("Command Station: Reading response...");
    RailcomMessage* msg = cs_manager.readMessage();

    if (msg != nullptr && msg->id == RailcomID::POM) {
        PomMessage* pom_msg = static_cast<PomMessage*>(msg);
        if (pom_msg->cvValue == CV_VALUE) {
            Serial.println("SUCCESS: In-circuit test passed!");
        } else {
            Serial.print("FAILURE: Incorrect CV value. Expected ");
            Serial.print(CV_VALUE);
            Serial.print(", got ");
            Serial.println(pom_msg->cvValue);
        }
    } else {
        Serial.println("FAILURE: Did not receive a valid POM response.");
    }
}

void loop() {
    // Keep tasks running to flush any remaining UART data etc.
    cs_sender.task();
    loco_sender.task();
}
