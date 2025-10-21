#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0);
RailcomReceiver receiver(&stream);
RailcomManager manager(sender, receiver);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    sender.begin();
    receiver.begin();

    Serial.println("Command Station Example (Refactored)");
    Serial.println("Enter commands:");
    Serial.println("  'p <addr> <cv>' - Send POM Read request");
}

void loop() {
    sender.task();

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\\n');
        input.trim();
        parseCommand(input);
    }

    RailcomMessage* msg = manager.readMessage();
    if (msg != nullptr) {
        // ... (message printing logic is unchanged)
    }
}

void parseCommand(String cmd) {
    if (cmd.startsWith("p")) {
        int addr = cmd.substring(2, cmd.indexOf(' ', 2)).toInt();
        int cv = cmd.substring(cmd.indexOf(' ', 2) + 1).toInt();

        uint8_t dcc_data[] = { (uint8_t)(addr >> 8), (uint8_t)addr, 0b11101100, (uint8_t)cv, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    } else {
        Serial.println("Unknown command");
    }
}
