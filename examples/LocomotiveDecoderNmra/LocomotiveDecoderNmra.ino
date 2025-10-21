#include <Arduino.h>
#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

HardwareUartStream stream(uart0, 0, 1);
RailcomSender sender(&stream, 0);
RailcomReceiver receiver(&stream);
RailcomManager manager(sender, receiver);

NmraDcc dcc;

void notifyDccSpeedPacket(uint16_t address, DCC_ADDR_TYPE addr_type, uint8_t speed, DCC_DIRECTION forward) {
    if (address == LOCOMOTIVE_ADDRESS) {
        Serial.println("Received DCC packet for my address. Queueing address broadcast.");

        // Queue the address broadcast to be sent in the next cutout
        manager.sendAddress(LOCOMOTIVE_ADDRESS);

        // Trigger the cutout. The DCC packet has already been received, so we
        // can just create the cutout without re-sending it.
        DCCMessage empty_msg;
        sender.send_dcc_with_cutout(empty_msg);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    dcc.begin(INPUT_PIN, 0);
    dcc.setSpeedPacketHandler(notifyDccSpeedPacket, true);

    sender.begin();
    receiver.begin();

    Serial.println("Locomotive Decoder (NMRA) Example");
}

void loop() {
    dcc.process();
    sender.task();
}
