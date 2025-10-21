#include <Arduino.h>
#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t ACCESSORY_ADDRESS = 100;

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0);
RailcomReceiver receiver(&stream);
RailcomManager manager(sender, receiver);

NmraDcc dcc;
bool turnoutThrown = false;

// This function is called by the NmraDcc library whenever a valid packet is received
void notifyDccAccPacket(uint16_t address, bool activate, uint8_t output, bool C, bool S) {
    if (address == ACCESSORY_ADDRESS) {
        turnoutThrown = activate;

        Serial.print("Received NmraDcc packet: Address=");
        Serial.print(address);
        Serial.print(", Thrown=");
        Serial.println(turnoutThrown);

        // Queue a status report to be sent in the next cutout
        uint8_t status = turnoutThrown ? 1 : 0;
        manager.sendStatus1(status);

        // Trigger the cutout so the message can be sent
        DCCMessage empty_msg; // An empty DCC packet for the cutout
        sender.send_dcc_with_cutout(empty_msg);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    dcc.begin(INPUT_PIN, 0);
    dcc.setAccPacketHandler(notifyDccAccPacket, true);

    sender.begin();
    receiver.begin();

    Serial.println("Accessory Decoder (NMRA) Example");
}

void loop() {
    // The DCC library and Railcom sender must be polled continuously
    dcc.process();
    sender.task();
}
