#include <Arduino.h>
#include "RailcomTx.h"
#include "RailcomRx.h"
#include "RP2040RailcomHardware.h"

RP2040RailcomHardware hardware(uart0, 0, 1, 1);
RailcomTx railcomTx(&hardware);
RailcomRx railcomRx(&hardware);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    railcomTx.begin();
    // railcomRx.begin(); No longer needed as hardware.begin() is called in railcomTx.begin()
    Serial.println("Command Station Example (Refactored)");
    Serial.println("Enter 'p <addr> <cv>' to send POM Read");
}

void parseCommand(String cmd) {
    if (cmd.startsWith("p")) {
        int addr = cmd.substring(2, cmd.indexOf(' ', 2)).toInt();
        int cv = cmd.substring(cmd.indexOf(' ', 2) + 1).toInt();

        uint8_t dcc_data[] = { (uint8_t)(addr >> 8), (uint8_t)addr, 0b11101100, (uint8_t)cv, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        railcomTx.send_dcc_with_cutout(dcc_msg);
    }
}

void loop() {
    railcomTx.task();

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\\n');
        input.trim();
        parseCommand(input);
    }

    RailcomMessage* msg = railcomRx.readMessage();
    if (msg != nullptr) {
        Serial.print("Received Message! ID: ");
        Serial.println((int)msg->id);
    }
}
