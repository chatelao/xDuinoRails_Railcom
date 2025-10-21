#include <AUnit.h>
#include "RailcomSender.h"
#include "RailcomReceiver.h"
#include "RailcomTxManager.h"
#include "RailcomRxManager.h"

// --- Test Environment Setup ---
RailcomSender cs_sender(uart0, 0, 1);
RailcomReceiver cs_receiver(uart0, 1);
RailcomTxManager cs_tx_manager(cs_sender);
RailcomRxManager cs_rx_manager(cs_receiver);

const uint16_t DECODER_ADDRESS = 4098;
RailcomSender decoder_sender(uart1, 4, 5);
RailcomReceiver decoder_receiver(uart1, 5);
RailcomTxManager decoder_tx_manager(decoder_sender);
// Note: We don't need a decoder_rx_manager for these tests

void setup() {
    Serial.begin(115200);
    while(!Serial);
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

void trigger_cutout() {
    DCCMessage dcc_msg((const uint8_t[]){0,0,0}, 3);
    cs_sender.begin_cutout_sequence(dcc_msg);
    delay(5);
    decoder_sender.task();
    delay(10);
}

// --- Test Cases ---

test(EndToEnd, pomRead) {
    decoder_tx_manager.sendPomResponse(151);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    assertEqual(static_cast<PomMessage*>(msg)->cvValue, 151);
}

test(EndToEnd, addressBroadcastLong) {
    decoder_tx_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg1 = cs_rx_manager.readMessage();
    assertNotNull(msg1);
    assertEqual((int)msg1->id, (int)RailcomID::ADR_HIGH);

    decoder_tx_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg2 = cs_rx_manager.readMessage();
    assertNotNull(msg2);
    assertEqual((int)msg2->id, (int)RailcomID::ADR_LOW);
}

test(EndToEnd, dynamicData) {
    decoder_tx_manager.sendDynamicData(5, 75);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::DYN);
    assertEqual(static_cast<DynMessage*>(msg)->subIndex, 5);
    assertEqual(static_cast<DynMessage*>(msg)->value, 75);
}

test(EndToEnd, serviceRequest) {
    decoder_tx_manager.sendServiceRequest(123, false);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    // The parser re-uses the AdrMessage struct for SRQs
    assertEqual((int)msg->id, (int)RailcomID::ADR_LOW);
    assertEqual(static_cast<AdrMessage*>(msg)->address, 123);
}
