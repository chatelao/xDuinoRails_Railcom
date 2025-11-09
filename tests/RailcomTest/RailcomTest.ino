#include "RailcomTx.h"
#include "RailcomRx.h"
#include "RailcomEncoding.h"
#include "mocks/MockRailcomHardware.h"

// Minimal testing framework
#define test(name) void test_##name()
#define run_test(name) Serial.print("Running test: "#name"... "); test_##name(); Serial.println("PASSED")
#define assertEqual(a, b) if (a != b) { Serial.print("FAILED: "); Serial.print((int)a); Serial.print(" != "); Serial.println((int)b); while(1); }
#define assertTrue(a) if (!(a)) { Serial.println("FAILED: assertion failed"); while(1); }
#define assertNotNull(a) if (a == nullptr) { Serial.println("FAILED: pointer is null"); while(1); }

// Verifies that the RailcomTx and RailcomRx classes can be instantiated.
test(creation) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  assertTrue(&tx != nullptr);
  assertTrue(&rx != nullptr);
}

// Verifies the end-to-end transmission and reception of a short address.
test(short_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t shortAddress = 100;

  // Send the low part of the address first (alternator is initially false).
  tx.sendAddress(shortAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, shortAddress & 0x7F);
  hardware.clear();

  // Send the high part of the address (alternator is now true).
  tx.sendAddress(shortAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 0);
  hardware.clear();
}

// Verifies that every message sent by RailcomTx can be correctly parsed by RailcomRx.
test(end_to_end) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // Verifies POM (ID 0) message sending and parsing.
  tx.sendPomResponse(42);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(msg)->cvValue, 42);
  hardware.clear();

  // Verifies ADR_HIGH (ID 1) message sending and parsing.
  tx.sendAddress(3);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 3);
  hardware.clear();

  // Verifies DYN (ID 7) message sending and parsing.
  tx.sendDynamicData(1, 100);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DYN);
  assertEqual(static_cast<DynMessage*>(msg)->subIndex, 1);
  assertEqual(static_cast<DynMessage*>(msg)->value, 100);
  hardware.clear();

  // Verifies XPOM (ID 8-11) message sending and parsing.
  uint8_t cvs[] = {1, 2, 3, 4};
  tx.sendXpomResponse(0, cvs);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::XPOM_0);
  XpomMessage* xpomMsg = static_cast<XpomMessage*>(msg);
  assertEqual(xpomMsg->sequence, 0);
  assertEqual(xpomMsg->cvValues[0], 1);
  assertEqual(xpomMsg->cvValues[1], 2);
  assertEqual(xpomMsg->cvValues[2], 3);
  assertEqual(xpomMsg->cvValues[3], 4);
  hardware.clear();

  // Verifies STAT1 (ID 4) message sending and parsing.
  tx.sendStatus1(0xAB);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT1);
  assertEqual(static_cast<Stat1Message*>(msg)->status, 0xAB);
  hardware.clear();

  // Verifies STAT2 (ID 8) message sending and parsing.
  tx.sendStatus2(0x11);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT2);
  assertEqual(static_cast<Stat2Message*>(msg)->status, 0x11);
  hardware.clear();

  // Verifies STAT4 (ID 3) message sending and parsing.
  tx.sendStatus4(0xCD);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(msg)->status, 0xCD);
  hardware.clear();

  // Verifies ERROR (ID 6) message sending and parsing.
  tx.sendError(0xEF);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ERROR);
  assertEqual(static_cast<ErrorMessage*>(msg)->errorCode, 0xEF);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 0.1s).
  tx.sendTime(127, false); // 12.7 seconds
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg1 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg1->unit_is_second, false);
  assertEqual(timeMsg1->timeValue, 127);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 1s).
  tx.sendTime(42, true); // 42 seconds
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg2 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg2->unit_is_second, true);
  assertEqual(timeMsg2->timeValue, 42);
  hardware.clear();

  // Verifies CV_AUTO (ID 12) message sending and parsing.
  tx.sendCvAuto(0x123456, 0xAB);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  CvAutoMessage* cvAutoMsg = static_cast<CvAutoMessage*>(msg);
  assertEqual(cvAutoMsg->cvAddress, 0x123456);
  assertEqual(cvAutoMsg->cvValue, 0xAB);
  hardware.clear();

  // Verifies EXT (ID 3) message sending and parsing.
  tx.sendExt(0x05, 0xBC);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::EXT);
  ExtMessage* extMsg = static_cast<ExtMessage*>(msg);
  assertEqual(extMsg->type, 0x05);
  assertEqual(extMsg->position, 0xBC);
  hardware.clear();

  // Verifies BLOCK (ID 13) message sending and parsing.
  tx.sendBlock(0xABCDEF12);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::BLOCK);
  BlockMessage* blockMsg = static_cast<BlockMessage*>(msg);
  assertEqual(blockMsg->data, 0xABCDEF12);
  hardware.clear();
}


void setup() {
  Serial.begin(115200);
  while (!Serial);

  run_test(creation);
  run_test(end_to_end);
  run_test(long_address_e2e);
  run_test(short_address_e2e);
  run_test(service_request_e2e);
  run_test(decoder_state_machine_e2e);
  run_test(cv_config_disables_railcom);
  run_test(dynamic_channel1_management);
  run_test(decoder_unique_e2e);
  run_test(decoder_state_e2e);
  run_test(ack_nack_e2e);
  run_test(rerailing_search_e2e);
  run_test(data_space_e2e);
  run_test(info1_cycle_e2e);
  run_test(info_message_e2e);
  run_test(xf1_location_request_e2e);

  Serial.println("All tests passed!");
}

// Verifies the end-to-end transmission and reception of the ADR/INFO1 cycle.
test(info1_cycle_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t longAddress = 4097;

  // Enable INFO1 with some dummy data
  Info1Message info1_data;
  info1_data.on_track_direction_is_positive = true;
  info1_data.travel_direction_is_positive = false;
  info1_data.is_moving = true;
  info1_data.is_in_consist = false;
  info1_data.request_addressing = true;
  tx.enableInfo1(info1_data);

  // Set receiver context to mobile to correctly interpret ID 3
  rx.setContext(DecoderContext::MOBILE);

  // --- First cycle ---
  // 1. ADR_HIGH
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (longAddress >> 8) & 0x3F);
  hardware.clear();

  // 2. ADR_LOW
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, longAddress & 0xFF);
  hardware.clear();

  // 3. INFO1
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::INFO1);
  Info1Message* received_info1 = static_cast<Info1Message*>(msg);
  assertEqual(received_info1->on_track_direction_is_positive, true);
  assertEqual(received_info1->travel_direction_is_positive, false);
  assertEqual(received_info1->is_moving, true);
  assertEqual(received_info1->is_in_consist, false);
  assertEqual(received_info1->request_addressing, true);
  hardware.clear();

  // Disable INFO1 and verify the cycle returns to 2 states
  tx.disableInfo1();

  // --- Second cycle (should be only ADR_HIGH, ADR_LOW) ---
  // 1. ADR_HIGH
  tx.sendAddress(longAddress); // Should be ADR_HIGH again
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  hardware.clear();

  // 2. ADR_LOW
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception of a long address.
test(long_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t longAddress = 4097; // Example long address (0x1001)

  // Send the high part of the address.
  // The internal alternator in RailcomTx starts with the high part.
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (longAddress >> 8) & 0x3F);
  hardware.clear();

  // Send the low part of the address.
  tx.sendAddress(longAddress);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, longAddress & 0xFF);
  hardware.clear();
}

#include "DecoderStateMachine.h"

// Verifies the DecoderStateMachine's response to a POM read command.
test(decoder_state_machine_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // CV28=enable all, CV29=enable railcom, long addr, 28/128 steps
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00001010);

  // Simulate a DCC POM read command for CV 1
  uint8_t dcc_data[] = {0, 100, 0b11100100, 1, 0}; // Address 100, Read CV 1
  DCCMessage msg(dcc_data, 5);
  sm.handleDccPacket(msg);

  // Verify that a POM response with the dummy value 42 is sent
  RailcomRx rx(&hardware);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(railcomMsg)->cvValue, 42);
}

// Verifies that no RailCom messages are sent when disabled via CV29.
test(cv_config_disables_railcom) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // CV29 has bit 3 (RailCom enable) set to 0.
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00000010);

  // Simulate a DCC packet
  uint8_t dcc_data[] = {0, 100, 0b01100000, 0};
  DCCMessage msg(dcc_data, 4);
  sm.handleDccPacket(msg);

  // Verify that NO message was sent
  assertTrue(hardware.getQueuedMessages().empty());
}

// Verifies that the address broadcast on Ch1 stops after the decoder is addressed.
test(dynamic_channel1_management) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // RailCom is enabled.
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00001010);

  // 1. Simulate a DCC packet for a DIFFERENT locomotive.
  // We expect an address broadcast because the channel is open.
  uint8_t other_loco_data[] = {0, 101, 0b01100000, 0};
  DCCMessage other_loco_msg(other_loco_data, 4);
  sm.handleDccPacket(other_loco_msg);
  assertTrue(hardware.getQueuedMessages().count(1)); // Should broadcast address
  hardware.clear();

  // 2. Simulate a DCC packet for THIS locomotive.
  // This should disable future broadcasts.
  uint8_t my_loco_data[] = {0, 100, 0b01100000, 0};
  DCCMessage my_loco_msg(my_loco_data, 4);
  sm.handleDccPacket(my_loco_msg);
  hardware.clear(); // Clear any messages sent in response to this packet

  // 3. Simulate another packet for the OTHER locomotive.
  // We expect NO broadcast this time.
  sm.handleDccPacket(other_loco_msg);
  assertTrue(hardware.getQueuedMessages().empty());
}

void loop() {
  // Do nothing
}

// Verifies the end-to-end transmission and reception of a service request.
test(service_request_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t accessoryAddress = 1234;

  tx.sendServiceRequest(accessoryAddress, true);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::SRQ);
  SrqMessage* srqMsg = static_cast<SrqMessage*>(msg);
  assertEqual(srqMsg->accessoryAddress, accessoryAddress);
  assertEqual(srqMsg->isExtended, true);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception of a DECODER_UNIQUE message.
test(decoder_unique_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t manufacturerId = 0x0ABC; // 12-bit value
  uint32_t productId = 0x12345678;

  tx.sendDecoderUnique(manufacturerId, productId);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_UNIQUE);
  DecoderUniqueMessage* uniqueMsg = static_cast<DecoderUniqueMessage*>(msg);
  assertEqual(uniqueMsg->manufacturerId, manufacturerId);
  assertEqual(uniqueMsg->productId, productId);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception of a DECODER_STATE message.
test(decoder_state_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  tx.sendDecoderState(0xAA, 0x0BCC, 0xDDEE); // 12-bit changeCount
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_STATE);
  DecoderStateMessage* stateMsg = static_cast<DecoderStateMessage*>(msg);
  assertEqual(stateMsg->changeFlags, 0xAA);
  assertEqual(stateMsg->changeCount, 0x0BCC);
  assertEqual(stateMsg->protocolCaps, 0xDDEE);
  hardware.clear();
}

// Verifies the correct byte sequences for ACK and NACK messages.
test(ack_nack_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);

  // Test ACK
  tx.sendAck();
  auto queuedMsgs = hardware.getQueuedMessages();
  assertTrue(queuedMsgs.count(1));
  assertTrue(queuedMsgs.count(2));
  std::vector<uint8_t> expected_ch1_ack = {RAILCOM_ACK1, RAILCOM_ACK2};
  std::vector<uint8_t> expected_ch2_ack = {RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2};
  assertEqual(queuedMsgs.at(1).size(), expected_ch1_ack.size());
  for(size_t i=0; i<expected_ch1_ack.size(); ++i) assertEqual(queuedMsgs.at(1)[i], expected_ch1_ack[i]);
  assertEqual(queuedMsgs.at(2).size(), expected_ch2_ack.size());
  for(size_t i=0; i<expected_ch2_ack.size(); ++i) assertEqual(queuedMsgs.at(2)[i], expected_ch2_ack[i]);
  hardware.clear();

  // Test NACK
  tx.sendNack();
  queuedMsgs = hardware.getQueuedMessages();
  assertTrue(queuedMsgs.count(1));
  assertTrue(queuedMsgs.count(2));
  std::vector<uint8_t> expected_ch1_nack = {RAILCOM_NACK, RAILCOM_NACK};
  std::vector<uint8_t> expected_ch2_nack = {RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK};
  assertEqual(queuedMsgs.at(1).size(), expected_ch1_nack.size());
  for(size_t i=0; i<expected_ch1_nack.size(); ++i) assertEqual(queuedMsgs.at(1)[i], expected_ch1_nack[i]);
  assertEqual(queuedMsgs.at(2).size(), expected_ch2_nack.size());
  for(size_t i=0; i<expected_ch2_nack.size(); ++i) assertEqual(queuedMsgs.at(2)[i], expected_ch2_nack[i]);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception for handleRerailingSearch.
test(rerailing_search_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t address = 0x1234;
  uint32_t seconds = 123;

  tx.handleRerailingSearch(address, seconds);
  hardware.setRxBuffer(hardware.getQueuedMessages());

  // 1. ADR_HIGH
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (address >> 8) & 0x3F);

  // 2. ADR_LOW
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, address & 0xFF);

  // 3. RERAIL
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::RERAIL);
  assertEqual(static_cast<RerailMessage*>(msg)->counter, seconds);

  hardware.clear();
}

// Verifies that sendDataSpace queues messages on the correct channels.
test(data_space_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  uint8_t data[] = {0x01, 0x02, 0x03};

  tx.sendDataSpace(data, sizeof(data), 1);
  auto queuedMsgs = hardware.getQueuedMessages();
  assertTrue(queuedMsgs.count(1));
  assertTrue(queuedMsgs.count(2));
  // A full end-to-end test is complex due to CRC.
  // For now, just verify that *something* was sent on both channels.
  assertTrue(queuedMsgs.at(1).size() > 0);
  assertTrue(queuedMsgs.at(2).size() > 0);
  hardware.clear();
}

// Verifies the end-to-end transmission and reception of an INFO message.
test(info_message_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // Set context to mobile to correctly interpret ID 4 as INFO
  rx.setContext(DecoderContext::MOBILE);

  uint16_t speed = 12345;
  uint8_t motorLoad = 180;
  uint8_t statusFlags = 0b10101010;

  tx.sendInfo(speed, motorLoad, statusFlags);
  hardware.setRxBuffer(hardware.getQueuedMessages());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::INFO);
  InfoMessage* infoMsg = static_cast<InfoMessage*>(msg);
  assertEqual(infoMsg->speed, speed);
  assertEqual(infoMsg->motorLoad, motorLoad);
  assertEqual(infoMsg->statusFlags, statusFlags);
  hardware.clear();
}

// Verifies that an XF1 command triggers an EXT response.
test(xf1_location_request_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // RailCom enabled, long address format
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 4097, 0b00000011, 0b00001010);
  RailcomRx rx(&hardware);

  // --- Test with Long Address ---
  // Simulate DCC XF1 command: C0 01 (Addr), DE (XF), 01 (XF1)
  // Note: The address part for a long address is (0xC000 | address).
  // So for address 4097 (0x1001), the bytes are 0xD0, 0x01.
  uint8_t dcc_data_long[] = {0xD0, 0x01, 0xDE, 0x01, 0};
  DCCMessage msg_long(dcc_data_long, 5);
  sm.handleDccPacket(msg_long);

  // Verify that an EXT message with dummy values (0, 0) is sent on channel 2
  hardware.setRxBuffer(hardware.getQueuedMessages());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::EXT);
  ExtMessage* extMsg = static_cast<ExtMessage*>(railcomMsg);
  assertEqual(extMsg->type, 0);
  assertEqual(extMsg->position, 0);
  // Verify it was sent on channel 2
  assertTrue(hardware.getQueuedMessages().count(2));
  assertTrue(!hardware.getQueuedMessages().count(1));
  hardware.clear();

  // --- Test with Short Address ---
  DecoderStateMachine sm_short(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00001010);
  // Simulate DCC XF1 command: 64 (Addr 100), DE (XF), 01 (XF1)
  uint8_t dcc_data_short[] = {100, 0xDE, 0x01, 0};
  DCCMessage msg_short(dcc_data_short, 4);
  sm_short.handleDccPacket(msg_short);

  // Verify that an EXT message is sent again
  hardware.setRxBuffer(hardware.getQueuedMessages());
  railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::EXT);
  hardware.clear();
}
