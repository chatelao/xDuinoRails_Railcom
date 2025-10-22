#include <ArduinoUnit.h>
#include "RailcomSender.h"
#include "RailcomRx.h"
#include "RailcomTxManager.h"

// --- Test Environment Setup ---
RailcomSender cs_sender(uart0, 0, 1);
RailcomRx cs_railcom_rx(uart0, 1);
RailcomTxManager cs_tx_manager(cs_sender);

const uint16_t DECODER_ADDRESS = 4098;
RailcomSender decoder_sender(uart1, 4, 5);
RailcomRx decoder_railcom_rx(uart1, 5);
RailcomTxManager decoder_tx_manager(decoder_sender);
// Note: We don't need a decoder_rx_manager for these tests

void setup() {
    Serial.begin(115200);
    while(!Serial);
    cs_sender.begin();
    cs_railcom_rx.begin();
    decoder_sender.begin();
    decoder_railcom_rx.begin();
}

void loop() {
    cs_sender.task();
    decoder_sender.task();
    Test::run();
}

void trigger_cutout() {
    DCCMessage dcc_msg((const uint8_t[]){0,0,0}, 3);
    cs_sender.send_dcc_with_cutout(dcc_msg);
    delay(5);
    decoder_sender.task();
    delay(10);
}

// --- Test Cases ---

test(EndToEnd_pomRead) {
    decoder_tx_manager.sendPomResponse(151);
    trigger_cutout();
    RailcomMessage* msg = cs_railcom_rx.readMessage();
    assertTrue(msg != nullptr);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    assertEqual((int)static_cast<PomMessage*>(msg)->cvValue, 151);
}

test(EndToEnd_addressBroadcastLong) {
    decoder_tx_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg1 = cs_railcom_rx.readMessage();
    assertTrue(msg1 != nullptr);
    assertEqual((int)msg1->id, (int)RailcomID::ADR_HIGH);

    decoder_tx_manager.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg2 = cs_railcom_rx.readMessage();
    assertTrue(msg2 != nullptr);
    assertEqual((int)msg2->id, (int)RailcomID::ADR_LOW);
}

test(EndToEnd_dynamicData) {
    decoder_tx_manager.sendDynamicData(5, 75);
    trigger_cutout();
    RailcomMessage* msg = cs_railcom_rx.readMessage();
    assertTrue(msg != nullptr);
    assertEqual((int)msg->id, (int)RailcomID::DYN);
    assertEqual((int)static_cast<DynMessage*>(msg)->subIndex, 5);
    assertEqual((int)static_cast<DynMessage*>(msg)->value, 75);
}

test(EndToEnd_serviceRequest) {
    decoder_tx_manager.sendServiceRequest(123, false);
    trigger_cutout();
    RailcomMessage* msg = cs_railcom_rx.readMessage();
    assertTrue(msg != nullptr);
    // The parser re-uses the AdrMessage struct for SRQs
    assertEqual((int)msg->id, (int)RailcomID::ADR_LOW);
    assertEqual((int)static_cast<AdrMessage*>(msg)->address, 123);
}

test(EndToEnd_rcn218Logon) {
    // 1. CS sends LOGON_ENABLE
    uint8_t logon_enable[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ENABLE | 1, 0x12, 0x34, 0x56 };
    DCCMessage logon_msg(logon_enable, sizeof(logon_enable));
    cs_sender.send_dcc_with_cutout(logon_msg);
    delay(15);

    // 2. Decoder should respond with DECODER_UNIQUE
    decoder_tx_manager.sendDecoderUnique(0x0123, 0x456789AB);
    decoder_sender.task();
    delay(10);

    // 3. CS should receive DECODER_UNIQUE
    RailcomMessage* unique_msg = cs_railcom_rx.readMessage();
    assertTrue(unique_msg != nullptr);
    assertEqual((int)unique_msg->id, (int)RailcomID::DECODER_UNIQUE);
    DecoderUniqueMessage* du_msg = static_cast<DecoderUniqueMessage*>(unique_msg);
    assertEqual((int)du_msg->manufacturerId, 0x0123);
    assertEqual((long)du_msg->productId, 0x456789ABL);

    // 4. CS sends LOGON_ASSIGN
    uint8_t logon_assign[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ASSIGN | 1, 0x23, 0x45, 0x67, 0x89, 0xAB, (DECODER_ADDRESS >> 8), (DECODER_ADDRESS & 0xFF) };
    DCCMessage assign_msg(logon_assign, sizeof(logon_assign));
    cs_sender.send_dcc_with_cutout(assign_msg);
    delay(15);

    // 5. Decoder should respond with DECODER_STATE
    decoder_tx_manager.sendDecoderState(0xCC, 0x1122, 0x3344);
    decoder_sender.task();
    delay(10);

    // 6. CS should receive DECODER_STATE
    RailcomMessage* state_msg = cs_railcom_rx.readMessage();
    assertTrue(state_msg != nullptr);
    assertEqual((int)state_msg->id, (int)RailcomID::DECODER_STATE);
    DecoderStateMessage* ds_msg = static_cast<DecoderStateMessage*>(state_msg);
    assertEqual((int)ds_msg->changeFlags, 0xCC);
    assertEqual((int)ds_msg->changeCount, 0x1122);
    assertEqual((int)ds_msg->protocolCaps, 0x3344);
}
