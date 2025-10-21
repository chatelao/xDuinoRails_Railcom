#include <Arduino.h>
#include "Railcom.h"
#include "RailcomManager.h"

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 0, 1);
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

    // Try to read and parse a message
    RailcomMessage* msg = manager.readMessage();
    if (msg != nullptr) {
        Serial.print("Received Message! ID: ");
        Serial.print((int)msg->id);

        if (msg->id == RailcomID::POM) {
            PomMessage* pomMsg = static_cast<PomMessage*>(msg);
            Serial.print(", CV Value: ");
            Serial.println(pomMsg->cvValue);
        } else {
            Serial.println();
        }
    }
}

void parseCommand(String cmd) {
    if (cmd.startsWith("p")) {
        int addr = cmd.substring(2, cmd.indexOf(' ', 2)).toInt();
        int cv = cmd.substring(cmd.indexOf(' ', 2) + 1).toInt();

        Serial.print("Sending POM Read to addr ");
        Serial.print(addr);
        Serial.print(" for CV ");
        Serial.println(cv);

        // For a POM read, the Railcom response is queued by the *other* device.
        // All we do here is send the DCC command that creates the cutout.
        uint8_t dcc_data[] = { (uint8_t)(addr >> 8), (uint8_t)addr, 0b11101100, (uint8_t)cv, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    } else {
        Serial.println("Unknown command");
    }
}
