#include <AUnit.h>
#include "Railcom.h"
#include "../mocks/MockRailcomTx.h"
#include "DecoderStateMachine.h"
#include "../mocks/MockDcc.h"

// RCN-218, Chapter 4.2.5 - Data Space 5
// A RailComPlus command station reads and displays the locomotive's
// full name (e.g., "DB Class 218") as stored in the decoder.

const uint16_t LOCO_ADDRESS = 123;
const char* LOCO_NAME = "DB Class 218";
const uint16_t LOCO_NAME_CV_START = 200;

test(DisplayingLocomotiveLongName, respondsWithDataSpace5) {
  MockRailcomTx railcom_tx;
  DecoderStateMachine state_machine(railcom_tx, DecoderType::LOCOMOTIVE, LOCO_ADDRESS);

  // First, write the locomotive name to the CVs that store it.
  for (int i = 0; i < strlen(LOCO_NAME); ++i) {
    state_machine.handleDccPacket(MockDcc::createPomWritePacket(LOCO_ADDRESS, LOCO_NAME_CV_START + i, LOCO_NAME[i]));
  }

  railcom_tx.clear();

  // The command station requests data from data space 5.
  uint8_t data[] = {RCN218::DCC_A_ADDRESS, RCN218::CMD_GET_DATA_START | 5, 0};
  data[2] = data[0] ^ data[1];
  DCCMessage dcc_msg(data, sizeof(data));

  state_machine.handleDccPacket(dcc_msg);

  // The decoder should respond with a sequence of INFO messages.
  // We'll reconstruct the name from the messages.
  String locoName = "";
  for (const auto& msg : railcom_tx.sentMessages) {
    if (msg.id == RailcomID::INFO && msg.data1 == 5) {
      for (const auto& byte : msg.data_space) {
        locoName += (char)byte;
      }
    }
  }

  assertEqual(locoName, LOCO_NAME);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  TestRunner::run();
}

void loop() {
}
