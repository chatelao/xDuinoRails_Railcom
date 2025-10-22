#include <AUnit.h>
#include "RailcomSender.h"
#include "RailcomReceiver.h"
#include "RailcomTxManager.h"
#include "RailcomRxManager.h"

// ... (Test Environment Setup is unchanged)

void setup() { /* ... */ }
void loop() { /* ... */ }
void trigger_cutout() { /* ... */ }

// --- Test Cases ---

/**
 * @test EndToEnd::pomRead
 * @brief Tests a complete POM Read cycle.
 * @see RCN-217, Section 5.1
 */
test(EndToEnd, pomRead) {
    decoder_tx_manager.sendPomResponse(151);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::POM);
    assertEqual(static_cast<PomMessage*>(msg)->cvValue, 151);
}

/**
 * @test EndToEnd::addressBroadcastLong
 * @brief Verifies the alternating broadcast of a long vehicle address.
 * @see RCN-217, Section 5.2
 */
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

/**
 * @test EndToEnd::dynamicData
 * @brief Tests the transmission of a Dynamic Variable (DYN) message.
 * @see RCN-217, Section 5.5
 */
test(EndToEnd, dynamicData) {
    decoder_tx_manager.sendDynamicData(5, 75);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::DYN);
    assertEqual(static_cast<DynMessage*>(msg)->subIndex, 5);
    assertEqual(static_cast<DynMessage*>(msg)->value, 75);
}

/**
 * @test EndToEnd::serviceRequest
 * @brief Verifies the transmission of a Service Request (SRQ).
 * @see RCN-217, Section 6.1
 */
test(EndToEnd, serviceRequest) {
    decoder_tx_manager.sendServiceRequest(123, false);
    trigger_cutout();
    RailcomMessage* msg = cs_rx_manager.readMessage();
    assertNotNull(msg);
    assertEqual((int)msg->id, (int)RailcomID::SRQ_MSG);
    assertEqual(static_cast<AdrMessage*>(msg)->address & 0x7FF, 123);
}
