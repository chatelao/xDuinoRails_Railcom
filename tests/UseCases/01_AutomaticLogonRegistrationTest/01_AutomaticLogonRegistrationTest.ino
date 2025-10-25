#include <ArduinoUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.hh"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-218, RailComPlus Logon Procedure
// A RailComPlus-enabled locomotive automatically registers with the command
// station using its unique ID, requiring no manual address entry.

const uint16_t MANUFACTURER_ID = 123;
const uint32_t PRODUCT_ID = 456789;

test(AutomaticLogonRegistration_registersWithCommandStation) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, 0, MANUFACTURER_ID, PRODUCT_ID);

  // 1. Command station enables logon
  DCCMessage logon_enable_msg = MockDcc::createDccAPacket(RCN218::CMD_LOGON_ENABLE | 0b11); // GG=11 for all decoders
  state_machine.handleDccPacket(logon_enable_msg);

  // 2. Decoder should respond with its unique ID
  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::DECODER_UNIQUE);
  assertEqual(railcom_tx.sentMessages[0].data1, MANUFACTURER_ID);
  assertEqual(railcom_tx.sentMessages[0].data2, PRODUCT_ID);

  railcom_tx.clear();

  // 3. Command station selects this decoder for logon (using part of its ID)
  uint8_t handle = (PRODUCT_ID >> 16) & 0x0F;
  DCCMessage select_msg = MockDcc::createDccAPacket(RCN218::CMD_SELECT | handle);
  state_machine.handleDccPacket(select_msg);

  // 4. Decoder should respond with its state
  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::DECODER_STATE);

  railcom_tx.clear();

  // 5. Command station assigns a temporary address
  uint16_t temp_address = 1234;
  DCCMessage assign_msg = MockDcc::createDccALogonAssignPacket(handle, temp_address);
  state_machine.handleDccPacket(assign_msg);

  // 6. State machine should now be registered with the temporary address
  // We can verify this by sending a speed packet to the new address and checking for a response.
  DCCMessage speed_msg = MockDcc::createSpeedPacket(temp_address, 100);
  state_machine.handleDccPacket(speed_msg);

  assertEqual(railcom_tx.sentMessages.size(), 1);
  assertEqual(railcom_tx.sentMessages[0].id, RailcomID::ADR);
  assertEqual(railcom_tx.sentMessages[0].data1, temp_address);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Test::run();
}

void loop() {
}
