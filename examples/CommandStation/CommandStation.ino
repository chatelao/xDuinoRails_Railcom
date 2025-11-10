#include <Arduino.h>
#include "RailcomTx.h"
#include "RailcomRx.h"
#include "RP2040RailcomHardware.h"

RP2040RailcomHardware hardware(uart0, 0, 1);
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

        // This example does not send a real DCC message.
        // It simulates the event by calling on_cutout_start() directly.
        railcomTx.on_cutout_start();
    }
}

void loop() {
    railcomTx.task();

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        parseCommand(input);
    }

    RailcomMessage* msg = railcomRx.read();
    if (msg != nullptr) {
        Serial.print("Received Message! ID: ");
        Serial.println((int)msg->id);
    }
}
