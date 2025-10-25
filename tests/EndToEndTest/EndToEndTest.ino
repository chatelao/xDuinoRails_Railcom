#include <ArduinoUnit.h>
#include "RailcomTx.h"
#include "RailcomRx.h"

// --- Test Environment Setup ---
RailcomTx cs_railcomTx(uart0, 0, 1);
RailcomRx cs_railcomRx(uart0, 1);

const uint16_t DECODER_ADDRESS = 4098;
RailcomTx decoder_railcomTx(uart1, 4, 5);
RailcomRx decoder_railcomRx(uart1, 5);

void setup() {
    Serial.begin(115200);
    while(!Serial);
    cs_railcomTx.begin();
    cs_railcomRx.begin();
    decoder_railcomTx.begin();
    decoder_railcomRx.begin();
    Test::run();
}

void loop() {
    cs_railcomTx.task();
    decoder_railcomTx.task();
}

void trigger_cutout() {
    DCCMessage dcc_msg((const uint8_t[]){0,0,0}, 3);
    cs_railcomTx.send_dcc_with_cutout(dcc_msg);
    delay(5);
    decoder_railcomTx.task();
    delay(10);
}

// --- Test Cases ---

test(EndToEnd_pomRead) {
    decoder_railcomTx.sendPomResponse(151);
    trigger_cutout();
    RailcomMessage* msg = cs_railcomRx.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    assertEqual(static_cast<PomMessage*>(msg)->cvValue, 151);
}

test(EndToEnd_addressBroadcastLong) {
    decoder_railcomTx.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg1 = cs_railcomRx.readMessage();
    assertNotNull(msg1);
    assertEqual((int)msg1->id, (int)RailcomID::ADR_HIGH);

    decoder_railcomTx.sendAddress(DECODER_ADDRESS);
    trigger_cutout();
    RailcomMessage* msg2 = cs_railcomRx.readMessage();
    assertNotNull(msg2);
    assertEqual((int)msg2->id, (int)RailcomID::ADR_LOW);
}

test(EndToEnd_dynamicData) {
    decoder_railcomTx.sendDynamicData(5, 75);
    trigger_cutout();
    RailcomMessage* msg = cs_railcomRx.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::DYN);
    assertEqual(static_cast<DynMessage*>(msg)->subIndex, 5);
    assertEqual(static_cast<DynMessage*>(msg)->value, 75);
}

test(EndToEnd_serviceRequest) {
    decoder_railcomTx.sendServiceRequest(123, false);
    trigger_cutout();
    RailcomMessage* msg = cs_railcomRx.readMessage();
    assertNotNull(msg);
    // The parser re-uses the AdrMessage struct for SRQs
    assertEqual((int)msg->id, (int)RailcomID::ADR_LOW);
    assertEqual(static_cast<AdrMessage*>(msg)->address, 123);
}

test(EndToEnd_rcn218Logon) {
    // 1. CS sends LOGON_ENABLE
    uint8_t logon_enable[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ENABLE | 1, 0x12, 0x34, 0x56 };
    DCCMessage logon_msg(logon_enable, sizeof(logon_enable));
    cs_railcomTx.send_dcc_with_cutout(logon_msg);
    delay(15);

    // 2. Decoder should respond with DECODER_UNIQUE
    decoder_railcomTx.sendDecoderUnique(0x0123, 0x456789AB);
    decoder_railcomTx.task();
    delay(10);

    // 3. CS should receive DECODER_UNIQUE
    RailcomMessage* unique_msg = cs_railcomRx.readMessage();
    assertNotNull(unique_msg);
    assertEqual((int)unique_msg->id, (int)RailcomID::DECODER_UNIQUE);
    DecoderUniqueMessage* du_msg = static_cast<DecoderUniqueMessage*>(unique_msg);
    assertEqual(du_msg->manufacturerId, 0x0123);
    assertEqual(du_msg->productId, 0x456789AB);

    // 4. CS sends LOGON_ASSIGN
    uint8_t logon_assign[] = { RCN218::DCC_A_ADDRESS, RCN218::CMD_LOGON_ASSIGN | 1, 0x23, 0x45, 0x67, 0x89, 0xAB, (DECODER_ADDRESS >> 8), (DECODER_ADDRESS & 0xFF) };
    DCCMessage assign_msg(logon_assign, sizeof(logon_assign));
    cs_railcomTx.send_dcc_with_cutout(assign_msg);
    delay(15);

    // 5. Decoder should respond with DECODER_STATE
    decoder_railcomTx.sendDecoderState(0xCC, 0x1122, 0x3344);
    decoder_railcomTx.task();
    delay(10);

    // 6. CS should receive DECODER_STATE
    RailcomMessage* state_msg = cs_railcomRx.readMessage();
    assertNotNull(state_msg);
    assertEqual((int)state_msg->id, (int)RailcomID::DECODER_STATE);
    DecoderStateMessage* ds_msg = static_cast<DecoderStateMessage*>(state_msg);
    assertEqual(ds_msg->changeFlags, 0xCC);
    assertEqual(ds_msg->changeCount, 0x1122);
    assertEqual(ds_msg->protocolCaps, 0x3344);
}
