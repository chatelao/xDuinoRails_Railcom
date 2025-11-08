#include <Arduino.h>
#include "RP2040RailcomHardware.h"
#include "RailcomTx.h"

// --- Pin Configuration ---
const uint TX_PIN = 0; // Pin for RailCom transmission
const uint PIO_PIN = 2; // Pin for DCC cutout signal
const uint RX_PIN = 1; // Pin for RailCom reception (not used in this example)

// --- Accessory Configuration ---
const uint16_t ACCESSORY_ADDRESS = 512;
const bool IS_EXTENDED_ADDRESS = false;

// --- Global Objects ---
RP2040RailcomHardware hardware(uart0, TX_PIN, PIO_PIN, RX_PIN);
RailcomTx railcomTx(&hardware);

unsigned long lastRequestTime = 0;
const long requestInterval = 5000; // 5 seconds

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        // Wait for serial to connect
    }
    Serial.println("Accessory Decoder with Service Request (SRQ) Example");

    railcomTx.begin();
}

void loop() {
    // Periodically send a Service Request
    unsigned long currentTime = millis();
    if (currentTime - lastRequestTime >= requestInterval) {
        lastRequestTime = currentTime;

        Serial.print("Sending SRQ for accessory address ");
        Serial.print(ACCESSORY_ADDRESS);
        Serial.println("...");

        railcomTx.sendServiceRequest(ACCESSORY_ADDRESS, IS_EXTENDED_ADDRESS);
    }

    // The task function must be called periodically to handle message sending
    railcomTx.task();
}
