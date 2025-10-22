#include <AUnit.h>
#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomSender.h"
#include "RailcomReceiver.h"
#include "RailcomTxManager.h"
#include "RailcomRxManager.h"
#include "DecoderStateMachine.h"

// --- Test Environment ---
RailcomSender cs_sender(uart0, 0, 1);
RailcomReceiver cs_receiver(uart0, 1);
RailcomTxManager cs_tx_manager(cs_sender);
RailcomRxManager cs_rx_manager(cs_receiver);

const uint16_t DECODER_ADDRESS = 4098;
RailcomSender decoder_sender(uart1, 4, 5);
RailcomReceiver decoder_receiver(uart1, 5);
RailcomTxManager decoder_tx_manager(decoder_sender);
DecoderStateMachine loco_sm(decoder_tx_manager, DecoderType::LOCOMOTIVE, DECODER_ADDRESS);

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

void run_test_cycle(const DCCMessage& dcc_msg) {
    loco_sm.handleDccPacket(dcc_msg);
    cs_sender.begin_cutout_sequence(dcc_msg);
    delay(5);
    decoder_sender.task();
    delay(10);
}

// --- Test Cases ---

test(Comprehensive, speedCommandShouldReturnAddress) {
    NmraDcc dccPacket;
    dccPacket.setSpeed(DECODER_ADDRESS, 12, 100, true);
    DCCMessage dcc_msg(dccPacket.getPacket(), dccPacket.getPacketSize());

    run_test_cycle(dcc_msg);

    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    // Any mobile packet should be acknowledged with an address broadcast
    assertTrue(msg->id == RailcomID::ADR_HIGH || msg->id == RailcomID::ADR_LOW);
}

test(Comprehensive, pomReadShouldReturnPomResponse) {
    NmraDcc dccPacket;
    dccPacket.setPomReadCv(DECODER_ADDRESS, 8);
    DCCMessage dcc_msg(dccPacket.getPacket(), dccPacket.getPacketSize());

    run_test_cycle(dcc_msg);

    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    assertEqual(static_cast<PomMessage*>(msg)->cvValue, 151); // Dummy CV8 value
}

test(Comprehensive, pomWriteShouldReturnAddress) {
    NmraDcc dccPacket;
    dccPacket.setPomWriteCv(DECODER_ADDRESS, 10, 20);
    DCCMessage dcc_msg(dccPacket.getPacket(), dccPacket.getPacketSize());

    run_test_cycle(dcc_msg);

    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    // POM writes are acknowledged with an address broadcast
    assertTrue(msg->id == RailcomID::ADR_HIGH || msg->id == RailcomID::ADR_LOW);
}
