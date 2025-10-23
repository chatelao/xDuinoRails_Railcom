#include <AUnit.h>
#include <Arduino.h>
#include "RailcomTx.h"
#include "RailcomRx.h"

// Define the pins for the self-test
const uint8_t TX_PIN = 0;
const uint8_t RX_PIN = 1;

RailcomTx railcomTx(uart0, TX_PIN, RX_PIN);
RailcomRx railcomRx(uart0, RX_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    railcomTx.begin();
    railcomRx.begin();
}

// Helper function to send and receive a RailCom message
RailcomMessage* sendAndReceive() {
    DCCMessage dcc_msg;
    railcomTx.send_dcc_with_cutout(dcc_msg);
    delay(5);
    railcomTx.task();

    unsigned long startTime = millis();
    RailcomMessage* msg = nullptr;
    while (millis() - startTime < 100) { // 100ms timeout
        msg = railcomRx.readMessage();
        if (msg) break;
        delay(1);
    }
    return msg;
}

void loop() {
    aunit::TestRunner::run();
}

// RCN-217, Chapter 7.2.1
test(PomMessage) {
    uint8_t cvValue = 42;
    railcomTx.sendPomResponse(cvValue);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::POM);
    PomMessage* pomMsg = static_cast<PomMessage*>(msg);
    assertEqual(pomMsg->cvValue, cvValue);
}

// RCN-217, Chapter 7.2.2
// Note: This test only covers short addresses to avoid statefulness issues
// with the long address alternator in RailcomTx across test runs.
test(AdrMessage_Short) {
    uint16_t address = 100;
    railcomTx.sendAddress(address);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::ADR_HIGH);
    AdrMessage* adrMsg = static_cast<AdrMessage*>(msg);
    assertEqual(adrMsg->address, address);
}

// RCN-217, Chapter 7.2.3
test(DynMessage) {
    uint8_t subIndex = 5;
    uint8_t value = 123;
    railcomTx.sendDynamicData(subIndex, value);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::DYN);
    DynMessage* dynMsg = static_cast<DynMessage*>(msg);
    assertEqual(dynMsg->subIndex, subIndex);
    assertEqual(dynMsg->value, value);
}

// RCN-217, Chapter 7.2.4
test(XpomMessage) {
    uint8_t sequence = 2;
    uint8_t cvValues[] = {10, 20, 30, 40};
    railcomTx.sendXpomResponse(sequence, cvValues);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::XPOM_2);
    XpomMessage* xpomMsg = static_cast<XpomMessage*>(msg);
    assertEqual(xpomMsg->sequence, sequence);
    assertEqual(xpomMsg->cvValues[0], cvValues[0]);
    assertEqual(xpomMsg->cvValues[1], cvValues[1]);
    assertEqual(xpomMsg->cvValues[2], cvValues[2]);
    assertEqual(xpomMsg->cvValues[3], cvValues[3]);
}

// RCN-218, Chapter 8.1
test(DecoderUniqueMessage) {
    uint16_t manufacturerId = 0xABCD;
    uint32_t productId = 0x12345678;
    railcomTx.sendDecoderUnique(manufacturerId, productId);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::DECODER_UNIQUE);
    DecoderUniqueMessage* uniqueMsg = static_cast<DecoderUniqueMessage*>(msg);
    assertEqual(uniqueMsg->manufacturerId, manufacturerId);
    assertEqual(uniqueMsg->productId, productId);
}

// RCN-218, Chapter 8.2
test(DecoderStateMessage) {
    uint8_t changeFlags = 0b10101010;
    uint16_t changeCount = 12345;
    uint16_t protocolCaps = 0b0101010101010101;
    railcomTx.sendDecoderState(changeFlags, changeCount, protocolCaps);

    RailcomMessage* msg = sendAndReceive();

    assertNotNull(msg);
    assertEqual(msg->id, RailcomID::DECODER_STATE);
    DecoderStateMessage* stateMsg = static_cast<DecoderStateMessage*>(msg);
    assertEqual(stateMsg->changeFlags, changeFlags);
    assertEqual(stateMsg->changeCount, changeCount);
    assertEqual(stateMsg->protocolCaps, protocolCaps);
}
