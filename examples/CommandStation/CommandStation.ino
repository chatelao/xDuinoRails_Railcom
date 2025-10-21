#include <Arduino.h>
#include "RailcomSender.h"
#include "RailcomReceiver.h"
#include "RailcomTxManager.h"
#include "RailcomRxManager.h"

RailcomSender sender(uart0, 0, 1);
RailcomReceiver receiver(uart0, 1);
RailcomTxManager txManager(sender);
RailcomRxManager rxManager(receiver);

void setup() {
    Serial.begin(115200);
    while (!Serial);
    sender.begin();
    receiver.begin();
    Serial.println("Command Station Example (Refactored)");
    Serial.println("Enter 'p <addr> <cv>' to send POM Read");
}

void parseCommand(String cmd) {
    if (cmd.startsWith("p")) {
        int addr = cmd.substring(2, cmd.indexOf(' ', 2)).toInt();
        int cv = cmd.substring(cmd.indexOf(' ', 2) + 1).toInt();

        uint8_t dcc_data[] = { (uint8_t)(addr >> 8), (uint8_t)addr, 0b11101100, (uint8_t)cv, 0};
        DCCMessage dcc_msg(dcc_data, sizeof(dcc_data));
        sender.send_dcc_with_cutout(dcc_msg);
    }
}

void loop() {
    sender.task();

    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\\n');
        input.trim();
        parseCommand(input);
    }

    RailcomMessage* msg = rxManager.readMessage();
    if (msg != nullptr) {
        Serial.print("Received Message! ID: ");
        Serial.println((int)msg->id);
    }
}
