#include <AUnit.h>
#include "Railcom.h"
#include "RailcomManager.h"
#include "HardwareUartStream.h"

// --- Test Environment Setup ---
// Command Station (CS) on UART0
HardwareUartStream cs_stream(uart0, 0, 1);
RailcomSender cs_sender(&cs_stream, 0); // PIO pin is TX pin
RailcomReceiver cs_receiver(&cs_stream);
RailcomManager cs_manager(cs_sender, cs_receiver);

// Decoder on UART1
const uint16_t DECODER_ADDRESS = 4098;
HardwareUartStream decoder_stream(uart1, 4, 5);
RailcomSender decoder_sender(&decoder_stream, 4); // PIO pin is TX pin
RailcomReceiver decoder_receiver(&decoder_stream);
RailcomManager decoder_manager(decoder_sender, decoder_receiver);


void setup() {
    Serial.begin(115200);
    while (!Serial);

    cs_sender.begin();
    cs_receiver.begin();
    decoder_sender.begin();
    decoder_receiver.begin();

    TestRunner::run();
}

void loop() {
    cs_sender.task();
    decoder_sender.task();
}

// ... (Helper functions and test cases remain the same, but will use the new objects)
// ...
