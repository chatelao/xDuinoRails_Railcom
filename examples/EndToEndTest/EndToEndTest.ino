#include <AUnit.h>
#include "Railcom.h"
#include "RailcomManager.h"

// This test suite verifies end-to-end communication using the RailcomManager.
// It simulates a Command Station and a Decoder on the same RP2040 using two UARTs.

// --- Test Environment Setup ---
// Command Station (CS) on UART0
RailcomSender cs_sender(uart0, 0, 1);
RailcomReceiver cs_receiver(uart0, 0, 1);
RailcomManager cs_manager(cs_sender, cs_receiver);

// Decoder on UART1
const uint16_t DECODER_ADDRESS = 4098;
RailcomSender decoder_sender(uart1, 4, 5);
RailcomReceiver decoder_receiver(uart1, 4, 5);
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
    // The task methods must be called continuously for the non-blocking sender to work.
    cs_sender.task();
    decoder_sender.task();
}

// --- Helper Functions ---
void trigger_cutout() {
    // Send a dummy DCC packet from the CS to create a cutout,
    // allowing the decoder to send its queued messages.
    DCCMessage dcc_msg((const uint8_t[]){DECODER_ADDRESS >> 8, (uint8_t)DECODER_ADDRESS, 0}, 3);
    cs_sender.send_dcc_with_cutout(dcc_msg);
    delay(5); // Give time for the PIO and ISR to run
    decoder_sender.task(); // Let the decoder send its queued message
    delay(10); // Give the UART time to transmit
}


// --- Test Cases ---

test(EndToEnd, pomRead) {
    const uint8_t CV_VALUE = 151;
    decoder_manager.sendPomResponse(CV_VALUE);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    PomMessage* pomMsg = static_cast<PomMessage*>(msg);
    assertEqual(pomMsg->cvValue, CV_VALUE);
}

test(EndToEnd, addressBroadcastShort) {
    decoder_manager.sendAddress(99);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::ADR_HIGH);
    AdrMessage* adrMsg = static_cast<AdrMessage*>(msg);
    assertEqual(adrMsg->address, 99);
}

test(EndToEnd, addressBroadcastLong) {
    // Long addresses are sent in two alternating parts.
    // Part 1: ADR_HIGH
    decoder_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg1 = cs_manager.readMessage();
    assertNotNull(msg1);
    assertEqual((int)msg1->id, (int)RailcomID::ADR_HIGH);
    AdrMessage* adrMsg1 = static_cast<AdrMessage*>(msg1);
    assertEqual(adrMsg1->address, (DECODER_ADDRESS >> 8) & 0x3F);

    // Part 2: ADR_LOW
    decoder_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg2 = cs_manager.readMessage();
    assertNotNull(msg2);
    assertEqual((int)msg2->id, (int)RailcomID::ADR_LOW);
    AdrMessage* adrMsg2 = static_cast<AdrMessage*>(msg2);
    assertEqual(adrMsg2->address, DECODER_ADDRESS & 0xFF);
}

test(EndToEnd, dynamicData) {
    const uint8_t SUB_INDEX = 5; // Fuel Level
    const uint8_t VALUE = 75;
    decoder_manager.sendDynamicData(SUB_INDEX, VALUE);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::DYN);
    DynMessage* dynMsg = static_cast<DynMessage*>(msg);
    assertEqual(dynMsg->subIndex, SUB_INDEX);
    assertEqual(dynMsg->value, VALUE);
}

test(EndToEnd, xpomRead) {
    const uint8_t CV_VALUES[] = {10, 20, 30, 40};
    decoder_manager.sendXpomResponse(1, CV_VALUES); // Sequence 1
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::XPOM_1);
    XpomMessage* xpomMsg = static_cast<XpomMessage*>(msg);
    assertEqual(xpomMsg->sequence, 1);
    assertEqual(xpomMsg->cvValues[0], 10);
    assertEqual(xpomMsg->cvValues[1], 20);
    assertEqual(xpomMsg->cvValues[2], 30);
    assertEqual(xpomMsg->cvValues[3], 40);
}

// --- Accessory Decoder (STAT) Test Cases ---

test(EndToEnd, serviceRequest) {
    const uint16_t ACCESSORY_ADDR = 123;
    decoder_manager.sendServiceRequest(ACCESSORY_ADDR, false);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    // We re-use ADR_LOW for SRQ messages in the parser for simplicity
    assertEqual((int)msg->id, (int)RailcomID::ADR_LOW);
    AdrMessage* srqMsg = static_cast<AdrMessage*>(msg);
    assertEqual(srqMsg->address, ACCESSORY_ADDR);
}

test(EndToEnd, status1Report) {
    const uint8_t STATUS = 0b10101010;
    decoder_manager.sendStatus1(STATUS);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::STAT1);
    // For simple 8-bit payloads, we can cast to POM message
    PomMessage* statMsg = static_cast<PomMessage*>(msg);
    assertEqual(statMsg->cvValue, STATUS);
}

test(EndToEnd, errorReport) {
    const uint8_t ERROR_CODE = 0x01; // Fuse defective
    decoder_manager.sendError(ERROR_CODE);
    trigger_cutout();

    RailcomMessage* msg = cs_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::ERROR);
    PomMessage* errorMsg = static_cast<PomMessage*>(msg);
    assertEqual(errorMsg->cvValue, ERROR_CODE);
}
