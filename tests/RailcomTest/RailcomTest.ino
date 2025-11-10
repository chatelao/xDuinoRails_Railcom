#include "RailcomTx.h"
#include "RailcomRx.h"
#include "RailcomEncoding.h"
#include "mocks/MockRailcomHardware.h"
#include "mocks/MockDcc.h"

// Minimal testing framework
#define test(name) void test_##name()
#define run_test(name) Serial.print("Running test: "#name"... "); test_##name(); Serial.println("PASSED")
#define assertEqual(a, b) if (a != b) { Serial.print("FAILED: "); Serial.print((int)a); Serial.print(" != "); Serial.println((int)b); while(1); }
#define assertTrue(a) if (!(a)) { Serial.println("FAILED: assertion failed"); while(1); }
#define assertNotNull(a) if (a == nullptr) { Serial.println("FAILED: pointer is null"); while(1); }

/**
 * @brief Verifies that the RailcomTx and RailcomRx classes can be instantiated.
 * @see RCN-217, generell
 */
test(creation) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  assertTrue(&tx != nullptr);
  assertTrue(&rx != nullptr);
}

/**
 * @brief Verifies the end-to-end transmission and reception of a short address.
 * @see RCN-217, Section 5.2
 */
test(short_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t shortAddress = 100;

  // Send the low part of the address first (alternator is initially false).
  tx.sendAddress(shortAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, shortAddress & 0x7F);
  hardware.clear();

  // Send the high part of the address (alternator is now true).
  tx.sendAddress(shortAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, 0);
  hardware.clear();
}

/**
 * @brief Verifies that every message sent by RailcomTx can be correctly parsed by RailcomRx.
 * @see RCN-217, Sections 5 and 6
 */
test(end_to_end) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // Verifies POM (ID 0) message sending and parsing.
  // See RCN-217, Section 5.1
  tx.sendPomResponse(42);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(msg)->cvValue, 42);
  hardware.clear();

  // Verifies DYN (ID 7) message sending and parsing.
  // See RCN-217, Section 5.5
  tx.sendDynamicData(1, 100);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DYN);
  assertEqual(static_cast<DynMessage*>(msg)->subIndex, 1);
  assertEqual(static_cast<DynMessage*>(msg)->value, 100);
  hardware.clear();

  // Verifies XPOM (ID 8-11) message sending and parsing.
  // See RCN-217, Section 5.6
  uint8_t cvs[] = {1, 2, 3, 4};
  tx.sendXpomResponse(0, cvs);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
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
  // See RCN-217, Section 6.3
  tx.sendStatus1(0xAB);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT1);
  assertEqual(static_cast<Stat1Message*>(msg)->status, 0xAB);
  hardware.clear();

  // Verifies STAT2 (ID 8) message sending and parsing.
  // See RCN-217, Section 6.9
  tx.sendStatus2(0x11);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT2);
  assertEqual(static_cast<Stat2Message*>(msg)->status, 0x11);
  hardware.clear();

  // Verifies STAT4 (ID 3) message sending and parsing.
  // See RCN-217, Section 6.4
  tx.sendStatus4(0xCD);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(msg)->status, 0xCD);
  hardware.clear();

  // Verifies ERROR (ID 6) message sending and parsing.
  // See RCN-217, Section 6.6
  tx.sendError(0xEF);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ERROR);
  assertEqual(static_cast<ErrorMessage*>(msg)->errorCode, 0xEF);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 0.1s).
  // See RCN-217, Section 6.5
  tx.sendTime(127, false); // 12.7 seconds
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg1 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg1->unit_is_second, false);
  assertEqual(timeMsg1->timeValue, 127);
  hardware.clear();

  // Verifies TIME (ID 5) message sending and parsing (unit is 1s).
  // See RCN-217, Section 6.5
  tx.sendTime(42, true); // 42 seconds
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  TimeMessage* timeMsg2 = static_cast<TimeMessage*>(msg);
  assertEqual(timeMsg2->unit_is_second, true);
  assertEqual(timeMsg2->timeValue, 42);
  hardware.clear();

  // Verifies CV_AUTO (ID 12) message sending and parsing.
  // See RCN-217, Section 5.7
  tx.sendCvAuto(0x123456, 0xAB);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  CvAutoMessage* cvAutoMsg = static_cast<CvAutoMessage*>(msg);
  assertEqual(cvAutoMsg->cvAddress, 0x123456);
  assertEqual(cvAutoMsg->cvValue, 0xAB);
  hardware.clear();

  // Verifies EXT (ID 3) message sending and parsing.
  // See RCN-217, Section 5.3
  tx.sendExt(0x05, 0xBC);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::EXT);
  ExtMessage* extMsg = static_cast<ExtMessage*>(msg);
  assertEqual(extMsg->type, 0x05);
  assertEqual(extMsg->position, 0xBC);
  hardware.clear();

  // Verifies BLOCK (ID 13) message sending and parsing.
  // See RCN-218 (Note: BLOCK is defined in RCN-218 but often used with RCN-217 context)
  tx.sendBlock(0xABCDEF12);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
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
  run_test(data_space_e2e_full);
  run_test(info1_cycle_e2e);
  run_test(info_message_e2e);
  run_test(xf1_location_request_e2e);
  run_test(xf2_rerailing_search_broadcast_e2e);
  run_test(xf3_cv_auto_e2e);
  run_test(accessory_decoder_e2e);
  run_test(logon_procedure_e2e);
  run_test(boundary_value_e2e);
  run_test(logon_error_cases_e2e);
  run_test(backoff_mechanism_e2e);
  run_test(data_space_request_e2e);

  Serial.println("All tests passed!");
}

/**
 * @brief Verifies the end-to-end transmission and reception of the ADR/INFO1 cycle.
 * @see RCN-217, Section 5.2.2
 */
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
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (longAddress >> 8) & 0x3F);
  hardware.clear();

  // 2. ADR_LOW
  tx.sendAddress(longAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, longAddress & 0xFF);
  hardware.clear();

  // 3. INFO1
  tx.sendAddress(longAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
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
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  hardware.clear();

  // 2. ADR_LOW
  tx.sendAddress(longAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  hardware.clear();
}

/**
 * @brief Verifies the end-to-end transmission and reception of a long address.
 * @see RCN-217, Section 5.2
 */
test(long_address_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t longAddress = 4097; // Example long address (0x1001)

  // Send the high part of the address.
  // The internal alternator in RailcomTx starts with the high part.
  tx.sendAddress(longAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(msg)->address, (longAddress >> 8) & 0x3F);
  hardware.clear();

  // Send the low part of the address.
  tx.sendAddress(longAddress);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(msg)->address, longAddress & 0xFF);
  hardware.clear();
}

#include "DecoderStateMachine.h"

/**
 * @brief Verifies the complete RCN-218 logon procedure.
 * @see RCN-218, Sections 2, 3.2, 3.6, 4.1, 4.2
 */
test(logon_procedure_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  uint16_t manufacturerId = 0x0ABC;
  uint32_t productId = 0x12345678;
  uint16_t newAddress = 5555;

  // Create a state machine for a RailComPlus-capable decoder
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 0, 0, 0b00001000, manufacturerId, productId);

  // --- 1. Central sends LOGON_ENABLE ---
  uint8_t logon_enable_data[] = {
    RCN218::DCC_A_ADDRESS,
    (uint8_t)RCN218::CMD_LOGON_ENABLE, // group=ALL
    0x12, 0x34, // ZID
    0x56,       // Session ID
    0           // XOR checksum placeholder
  };
  logon_enable_data[5] = logon_enable_data[0] ^ logon_enable_data[1] ^ logon_enable_data[2] ^ logon_enable_data[3] ^ logon_enable_data[4];
  DCCMessage logon_enable_msg(logon_enable_data, sizeof(logon_enable_data));
  sm.handleDccPacket(logon_enable_msg);

  // --- 2. Decoder responds with DECODER_UNIQUE ---
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_UNIQUE);
  DecoderUniqueMessage* uniqueMsg = static_cast<DecoderUniqueMessage*>(msg);
  assertEqual(uniqueMsg->manufacturerId, manufacturerId);
  assertEqual(uniqueMsg->productId, productId);
  hardware.clear();

  // --- 3. Central sends LOGON_ASSIGN ---
  uint8_t logon_assign_data[] = {
    RCN218::DCC_A_ADDRESS,
    (uint8_t)(RCN218::CMD_LOGON_ASSIGN | ((manufacturerId >> 8) & 0x0F)),
    (uint8_t)(manufacturerId & 0xFF),
    (uint8_t)(productId >> 24),
    (uint8_t)(productId >> 16),
    (uint8_t)(productId >> 8),
    (uint8_t)(productId & 0xFF),
    (uint8_t)(newAddress >> 8),
    (uint8_t)(newAddress & 0xFF),
    0 // XOR checksum placeholder
  };
  uint8_t checksum = 0;
  for(int i=0; i<9; i++) { checksum ^= logon_assign_data[i]; }
  logon_assign_data[9] = checksum;
  DCCMessage logon_assign_msg(logon_assign_data, sizeof(logon_assign_data));
  sm.handleDccPacket(logon_assign_msg);

  // --- 4. Decoder responds with DECODER_STATE ---
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_STATE);
  // We can't check the content as they are dummy values, but we confirm the type.
  hardware.clear();
}

/**
 * @brief Verifies the DecoderStateMachine's response to a POM read command.
 * @see RCN-217, Section 5.1.1
 */
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
  tx.on_cutout_start();
  RailcomRx rx(&hardware);
  hardware.setRxBuffer(hardware.getSentBytes());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::POM);
  assertEqual(static_cast<PomMessage*>(railcomMsg)->cvValue, 42);
}

/**
 * @brief Verifies that no RailCom messages are sent when disabled via CV29.
 * @see RCN-217, Section 4.2.2
 */
test(cv_config_disables_railcom) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // CV29 has bit 3 (RailCom enable) set to 0.
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00000010);

  // Simulate a DCC packet
  uint8_t dcc_data[] = {0, 100, 0b01100000, 0};
  DCCMessage msg(dcc_data, 4);
  sm.handleDccPacket(msg);
  tx.on_cutout_start();

  // Verify that NO message was sent
  assertTrue(hardware.getSentBytes().empty());
}

/**
 * @brief Verifies that the address broadcast on Ch1 stops after the decoder is addressed.
 * @see RCN-217, Section 5.2.1
 */
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
  tx.on_cutout_start();
  assertTrue(!hardware.getSentBytes().empty()); // Should broadcast address
  hardware.clear();

  // 2. Simulate a DCC packet for THIS locomotive.
  // This should disable future broadcasts.
  uint8_t my_loco_data[] = {0, 100, 0b01100000, 0};
  DCCMessage my_loco_msg(my_loco_data, 4);
  sm.handleDccPacket(my_loco_msg);
  tx.on_cutout_start();
  hardware.clear(); // Clear any messages sent in response to this packet

  // 3. Simulate another packet for the OTHER locomotive.
  // We expect NO broadcast this time.
  sm.handleDccPacket(other_loco_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());
}

/**
 * @brief Verifies that a DCC Data Space Read command triggers the correct Data Space response.
 * @see RCN-218, Section 4.3
 */
test(data_space_request_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  uint16_t address = 4097;
  uint16_t manufacturerId = 0x0ABC;
  uint32_t productId = 0x12345678;

  // RailCom enabled
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, address, 0, 0b00001000, manufacturerId, productId);

  // --- Simulate a DCC Data Space Read for Data Space 5 (Loco Name) ---
  uint8_t dcc_data[] = {0xD0, 0x01, 0xED, 0x50, 0};
  DCCMessage dcc_msg(dcc_data, 5);
  sm.handleDccPacket(dcc_msg);
  tx.on_cutout_start();

  // --- Verify the Response ---
  // Expected data from constructor: { 'D', 'B', ' ', 'C', 'l', 'a', 's', 's', ' ', '2', '1', '8' }
  std::vector<uint8_t> expected_data_full = { 'D', 'B', ' ', 'C', 'l', 'a', 's', 's', ' ', '2', '1', '8' };

  std::vector<uint8_t> crc_buffer;
  crc_buffer.push_back(expected_data_full.size());
  crc_buffer.insert(crc_buffer.end(), expected_data_full.begin(), expected_data_full.end());
  uint8_t expected_crc = RailcomEncoding::crc8(crc_buffer.data(), crc_buffer.size(), 5);

  std::vector<uint8_t> expected_raw_bytes;
  expected_raw_bytes.push_back(RailcomEncoding::encode4of8(expected_data_full.size()));
  for (uint8_t byte : expected_data_full) {
    expected_raw_bytes.push_back(RailcomEncoding::encode4of8(byte));
  }
  expected_raw_bytes.push_back(RailcomEncoding::encode4of8(expected_crc));

  const auto& sent_bytes = hardware.getSentBytes();
  assertEqual(sent_bytes.size(), expected_raw_bytes.size());
  for (size_t i = 0; i < sent_bytes.size(); ++i) {
    assertEqual(sent_bytes[i], expected_raw_bytes[i]);
  }
  hardware.clear();
}

void loop() {
  // Do nothing
}

/**
 * @brief Verifies the end-to-end transmission and reception of a service request.
 * @see RCN-217, Section 6.1
 */
test(service_request_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t accessoryAddress = 1234;

  tx.sendServiceRequest(accessoryAddress, true);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::SRQ);
  SrqMessage* srqMsg = static_cast<SrqMessage*>(msg);
  assertEqual(srqMsg->accessoryAddress, accessoryAddress);
  assertEqual(srqMsg->isExtended, true);
  hardware.clear();
}

/**
 * @brief Verifies the end-to-end transmission and reception of a DECODER_UNIQUE message.
 * @see RCN-218, Section 4.1
 */
test(decoder_unique_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t manufacturerId = 0x0ABC; // 12-bit value
  uint32_t productId = 0x12345678;

  tx.sendDecoderUnique(manufacturerId, productId);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_UNIQUE);
  DecoderUniqueMessage* uniqueMsg = static_cast<DecoderUniqueMessage*>(msg);
  assertEqual(uniqueMsg->manufacturerId, manufacturerId);
  assertEqual(uniqueMsg->productId, productId);
  hardware.clear();
}

/**
 * @brief Verifies the end-to-end transmission and reception of a DECODER_STATE message.
 * @see RCN-218, Section 4.2
 */
test(decoder_state_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  tx.sendDecoderState(0xAA, 0x0BCC, 0xDDEE); // 12-bit changeCount
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::DECODER_STATE);
  DecoderStateMessage* stateMsg = static_cast<DecoderStateMessage*>(msg);
  assertEqual(stateMsg->changeFlags, 0xAA);
  assertEqual(stateMsg->changeCount, 0x0BCC);
  assertEqual(stateMsg->protocolCaps, 0xDDEE);
  hardware.clear();
}

/**
 * @brief Verifies the correct byte sequences for ACK and NACK messages.
 * @see RCN-217, Section 2.5
 */
test(ack_nack_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);

  // Test ACK
  tx.sendAck();
  tx.on_cutout_start();
  auto sentBytes = hardware.getSentBytes();
  std::vector<uint8_t> expected_ack = {RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2, RAILCOM_ACK1, RAILCOM_ACK2};
  assertEqual(sentBytes.size(), expected_ack.size());
  for(size_t i=0; i<expected_ack.size(); ++i) assertEqual(sentBytes[i], expected_ack[i]);
  hardware.clear();

  // Test NACK
  tx.sendNack();
  tx.on_cutout_start();
  sentBytes = hardware.getSentBytes();
  std::vector<uint8_t> expected_nack = {RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK, RAILCOM_NACK};
  assertEqual(sentBytes.size(), expected_nack.size());
  for(size_t i=0; i<expected_nack.size(); ++i) assertEqual(sentBytes[i], expected_nack[i]);
  hardware.clear();
}

/**
 * @brief Verifies the end-to-end transmission and reception for handleRerailingSearch.
 * @see RCN-217, Section 5.2.3
 */
test(rerailing_search_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;
  uint16_t address = 0x1234;
  uint32_t seconds = 123;

  tx.handleRerailingSearch(address, seconds);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());

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

/**
 * @brief Verifies that an XF3 command correctly starts and stops the CV-Auto broadcast.
 * @see RCN-217, Section 5.7
 */
test(xf3_cv_auto_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  uint16_t address = 100;
  // RailCom enabled
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, address, 0, 0b00001000);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // --- 1. Trigger XF3 to START the broadcast ---
  // DCC packet: 100 (Addr), DE (XF), 03 (XF3)
  uint8_t dcc_data[] = {100, 0xDE, 0x03, 0};
  DCCMessage dcc_msg(dcc_data, 4);
  sm.handleDccPacket(dcc_msg);
  tx.on_cutout_start();

  // Verify no message is sent immediately
  assertTrue(hardware.getSentBytes().empty());

  // --- 2. Call task() to send the first CV ---
  sm.task();
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvAddress, 1);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvValue, 10);
  hardware.clear();

  // --- 3. Call task() again for the second CV ---
  sm.task();
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::CV_AUTO);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvAddress, 8);
  assertEqual(static_cast<CvAutoMessage*>(msg)->cvValue, 155);
  hardware.clear();

  // --- 4. Trigger XF3 again to STOP the broadcast ---
  sm.handleDccPacket(dcc_msg);
  tx.on_cutout_start();

  // --- 5. Call task() again and verify no message is sent ---
  sm.task();
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());
}

/**
 * @brief Verifies the full end-to-end transmission, reception, and parsing of a Data Space message.
 * @see RCN-218, Section 4.3
 */
test(data_space_e2e_full) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  uint8_t data[] = {0x01, 0x02, 0x03, 0xAA, 0xBB}; // len=5
  uint8_t dataSpaceNum = 5;

  // 1. TX sends the data
  tx.sendDataSpace(data, sizeof(data), dataSpaceNum);
  tx.on_cutout_start();

  // 2. RX receives the data
  hardware.setRxBuffer(hardware.getSentBytes());
  rx.expectDataSpaceResponse(dataSpaceNum);
  RailcomMessage* msg_raw = rx.read();

  // 3. Verify the received data
  assertNotNull(msg_raw);
  DataSpaceMessage* msg = static_cast<DataSpaceMessage*>(msg_raw);

  assertEqual(msg->dataSpaceNum, dataSpaceNum);
  assertEqual(msg->len, sizeof(data));
  assertTrue(msg->crc_ok);
  for (size_t i = 0; i < sizeof(data); ++i) {
    assertEqual(msg->data[i], data[i]);
  }

  hardware.clear();
}


/**
 * @brief Verifies the end-to-end transmission and reception of an INFO message.
 * @see RCN-217, Section 5.4
 */
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
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::INFO);
  InfoMessage* infoMsg = static_cast<InfoMessage*>(msg);
  assertEqual(infoMsg->speed, speed);
  assertEqual(infoMsg->motorLoad, motorLoad);
  assertEqual(infoMsg->statusFlags, statusFlags);
  hardware.clear();
}

/**
 * @brief Verifies that an XF1 command triggers an EXT response.
 * @see RCN-217, Section 5.3.1
 */
test(xf1_location_request_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // RailCom enabled, long address format
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 4097, 0b00000011, 0b00001010);
  RailcomRx rx(&hardware);

  // --- Test with Long Address ---
  uint8_t dcc_data_long[] = {0xD0, 0x01, 0xDE, 0x01, 0};
  DCCMessage msg_long(dcc_data_long, 5);
  sm.handleDccPacket(msg_long);
  tx.on_cutout_start();

  hardware.setRxBuffer(hardware.getSentBytes());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::EXT);
  ExtMessage* extMsg = static_cast<ExtMessage*>(railcomMsg);
  assertEqual(extMsg->type, 0);
  assertEqual(extMsg->position, 0);
  hardware.clear();

  // --- Test with Short Address ---
  DecoderStateMachine sm_short(tx, DecoderType::LOCOMOTIVE, 100, 0b00000011, 0b00001010);
  uint8_t dcc_data_short[] = {100, 0xDE, 0x01, 0};
  DCCMessage msg_short(dcc_data_short, 4);
  sm_short.handleDccPacket(msg_short);
  tx.on_cutout_start();

  hardware.setRxBuffer(hardware.getSentBytes());
  railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::EXT);
  hardware.clear();
}

/**
 * @brief Verifies that a broadcast XF2 command triggers the rerailing search response.
 * @see RCN-217, Section 5.2.3
 */
test(xf2_rerailing_search_broadcast_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  // RailCom enabled, use a long address for the test
  uint16_t address = 4097;
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, address, 0b00000011, 0b00001010);
  RailcomRx rx(&hardware);

  uint8_t dcc_data[] = {0x00, 0xDE, 0x02, 0};
  DCCMessage msg(dcc_data, 4);
  sm.handleDccPacket(msg);
  tx.on_cutout_start();

  hardware.setRxBuffer(hardware.getSentBytes());

  // 1. ADR_HIGH
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::ADR_HIGH);
  assertEqual(static_cast<AdrMessage*>(railcomMsg)->address, (address >> 8) & 0x3F);

  // 2. ADR_LOW
  railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::ADR_LOW);
  assertEqual(static_cast<AdrMessage*>(railcomMsg)->address, address & 0xFF);

  // 3. RERAIL (TIME)
  railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::RERAIL);
  assertTrue(static_cast<RerailMessage*>(railcomMsg)->counter >= 0);

  hardware.clear();
}

/**
 * @brief Verifies the DecoderStateMachine's response to an accessory command.
 * @see RCN-217, Section 6.3 and 6.4
 */
test(accessory_decoder_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  uint16_t address = 200;
  // CV29 enables RailCom.
  DecoderStateMachine sm(tx, DecoderType::ACCESSORY, address, 0, 0b00001000);

  // Set receiver context to stationary to correctly interpret ID 3
  rx.setContext(DecoderContext::STATIONARY);

  // --- 1. Activate output 1 ---
  DCCMessage msg_on = MockDcc::createAccessoryDccMessage(address, true, 1);
  sm.handleDccPacket(msg_on);
  tx.on_cutout_start();

  hardware.setRxBuffer(hardware.getSentBytes());
  RailcomMessage* railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(railcomMsg)->status, 0b00000010); // Output 1 corresponds to bit 1
  hardware.clear();

  // --- 2. Deactivate output 1 ---
  DCCMessage msg_off = MockDcc::createAccessoryDccMessage(address, false, 1);
  sm.handleDccPacket(msg_off);
  tx.on_cutout_start();

  hardware.setRxBuffer(hardware.getSentBytes());
  railcomMsg = rx.read();
  assertNotNull(railcomMsg);
  assertEqual(railcomMsg->id, RailcomID::STAT4);
  assertEqual(static_cast<Stat4Message*>(railcomMsg)->status, 0b00000000);
  hardware.clear();
}

/**
 * @brief Verifies end-to-end communication with boundary (min/max) values for various messages.
 */
test(boundary_value_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  RailcomRx rx(&hardware);
  RailcomMessage* msg;

  // --- Test short address boundaries ---
  tx.sendAddress(1);
  tx.sendAddress(1);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  // Note: with the new queueing logic, we get ADR_HIGH first
  assertEqual(msg->id, RailcomID::ADR_HIGH);
  hardware.clear();


  // --- Test TIME max value ---
  tx.sendTime(127, true); // Max time value in seconds
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::TIME);
  assertEqual(static_cast<TimeMessage*>(msg)->timeValue, 127);
  assertEqual(static_cast<TimeMessage*>(msg)->unit_is_second, true);
  hardware.clear();

  // --- Test EXT max values ---
  tx.sendExt(7, 255); // Max type and position
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::EXT);
  assertEqual(static_cast<ExtMessage*>(msg)->type, 7);
  assertEqual(static_cast<ExtMessage*>(msg)->position, 255);
  hardware.clear();

  // --- Test SRQ boundaries ---
  tx.sendServiceRequest(1, false);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::SRQ);
  assertEqual(static_cast<SrqMessage*>(msg)->accessoryAddress, 1);
  hardware.clear();

  tx.sendServiceRequest(2047, true);
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  msg = rx.read();
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::SRQ);
  assertEqual(static_cast<SrqMessage*>(msg)->accessoryAddress, 2047);
  hardware.clear();

  // --- Test Rerailing search time capping ---
  tx.handleRerailingSearch(100, 300); // 300 > MAX_RERAIL_SECONDS (255)
  tx.on_cutout_start();
  hardware.setRxBuffer(hardware.getSentBytes());
  rx.read(); // ADR_HIGH
  rx.read(); // ADR_LOW
  msg = rx.read(); // RERAIL
  assertNotNull(msg);
  assertEqual(msg->id, RailcomID::RERAIL);
  assertEqual(static_cast<RerailMessage*>(msg)->counter, 255);
  hardware.clear();
}

/**
 * @brief Verifies that the DecoderStateMachine correctly handles error cases in the logon procedure.
 * @see RCN-218, Section 2
 */
test(logon_error_cases_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);

  uint16_t manufacturerId = 0x0ABC;
  uint32_t productId = 0x12345678;
  uint16_t newAddress = 5555;

  // --- 1. Test LOGON_ASSIGN with wrong Unique ID ---
  DecoderStateMachine sm_wrong_id(tx, DecoderType::LOCOMOTIVE, 0, 0, 0b00001000, manufacturerId, productId);
  uint8_t logon_enable_data[] = { RCN218::DCC_A_ADDRESS, (uint8_t)RCN218::CMD_LOGON_ENABLE, 0, 0, 0, 0 };
  DCCMessage logon_enable_msg(logon_enable_data, sizeof(logon_enable_data));
  sm_wrong_id.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  hardware.clear(); // Clear the DECODER_UNIQUE response

  uint32_t wrongProductId = 0x87654321;
  uint8_t logon_assign_wrong_id_data[] = {
    RCN218::DCC_A_ADDRESS,
    (uint8_t)(RCN218::CMD_LOGON_ASSIGN | ((manufacturerId >> 8) & 0x0F)),
    (uint8_t)(manufacturerId & 0xFF),
    (uint8_t)(wrongProductId >> 24), (uint8_t)(wrongProductId >> 16), (uint8_t)(wrongProductId >> 8), (uint8_t)(wrongProductId & 0xFF),
    (uint8_t)(newAddress >> 8), (uint8_t)(newAddress & 0xFF),
    0
  };
  DCCMessage logon_assign_wrong_id_msg(logon_assign_wrong_id_data, sizeof(logon_assign_wrong_id_data));

  sm_wrong_id.handleDccPacket(logon_assign_wrong_id_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());

  // --- 2. Test LOGON_ASSIGN in wrong state (IDLE) ---
  DecoderStateMachine sm_wrong_state(tx, DecoderType::LOCOMOTIVE, 0, 0, 0b00001000, manufacturerId, productId);
   uint8_t logon_assign_data[] = {
    RCN218::DCC_A_ADDRESS,
    (uint8_t)(RCN218::CMD_LOGON_ASSIGN | ((manufacturerId >> 8) & 0x0F)),
    (uint8_t)(manufacturerId & 0xFF),
    (uint8_t)(productId >> 24), (uint8_t)(productId >> 16), (uint8_t)(productId >> 8), (uint8_t)(productId & 0xFF),
    (uint8_t)(newAddress >> 8), (uint8_t)(newAddress & 0xFF),
    0
  };
  DCCMessage logon_assign_msg(logon_assign_data, sizeof(logon_assign_data));

  sm_wrong_state.handleDccPacket(logon_assign_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());
}

/**
 * @brief Verifies the RCN-218 backoff mechanism for collision avoidance.
 * @see RCN-218, Section 7.2
 */
test(backoff_mechanism_e2e) {
  MockRailcomHardware hardware;
  RailcomTx tx(&hardware);
  DecoderStateMachine sm(tx, DecoderType::LOCOMOTIVE, 0, 0, 0b00001000, 0x0ABC, 0x12345678);

  uint8_t logon_enable_data[] = { RCN218::DCC_A_ADDRESS, (uint8_t)RCN218::CMD_LOGON_ENABLE, 0, 0, 0, 0 };
  DCCMessage logon_enable_msg(logon_enable_data, sizeof(logon_enable_data));

  // 1. First LOGON_ENABLE: Decoder should respond.
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(!hardware.getSentBytes().empty());
  hardware.clear();

  // 2. Second LOGON_ENABLE (no assignment in between): Decoder should NOT respond (backoff=1).
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());

  // 3. Third LOGON_ENABLE: Decoder should respond again (backoff counter is done).
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(!hardware.getSentBytes().empty());
  hardware.clear();

  // 4. Fourth LOGON_ENABLE: Should NOT respond (backoff=2).
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());

  // 5. Fifth LOGON_ENABLE: Should NOT respond (backoff=2).
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(hardware.getSentBytes().empty());

  // 6. Sixth LOGON_ENABLE: Should respond again.
  sm.handleDccPacket(logon_enable_msg);
  tx.on_cutout_start();
  assertTrue(!hardware.getSentBytes().empty());
  hardware.clear();
}
