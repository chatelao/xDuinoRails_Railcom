#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-217, Chapter 7.2.2 - Message Type ADR
// A locomotive is placed on the track and its address is broadcast,
// allowing a detector to identify it.

const uint16_t LOCOMOTIVE_ADDRESS = 3;

test(BasicLocomotiveIdentification, sendsAdrOnIdle) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);

  // When a locomotive is idle, it should broadcast its address.
  // We'll simulate this by not sending any DCC packets and just calling the update method.
  state_machine.update();

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::ADR);
  assertEqual(railcom_tx.sentMessages[0].data1, LOCOMOTIVE_ADDRESS);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
