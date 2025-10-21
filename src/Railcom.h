#ifndef RAILCOM_H
#define RAILCOM_H

#include <Arduino.h>
#include <vector>
#include "hardware/pio.h"
#include "hardware/uart.h"

class DCCMessage {
public:
  DCCMessage();
  DCCMessage(const uint8_t* data, size_t len);
  uint16_t getAddress() const;
  uint8_t getCommand() const;
  const uint8_t* getData() const;
  size_t getLength() const;

private:
  uint8_t _data[8];
  size_t _len;
};

class RailcomSender {
public:
  RailcomSender(uart_inst_t* uart, uint txPin, uint rxPin);
  void begin();
  void end();
  void send_dcc_async(const DCCMessage& msg);

private:
  void pio_init();
  static void pio_irq_handler();

  uart_inst_t* _uart;
  uint _txPin;
  uint _rxPin;
  PIO _pio;
  uint _sm;
  static RailcomSender* _instance;
  static DCCMessage _msg;
};

class RailcomReceiver {
public:
  RailcomReceiver(uart_inst_t* uart, uint txPin, uint rxPin);
  void begin();
  void end();
  bool read_response(uint8_t* buffer, size_t len, uint timeout_ms);
  DCCMessage parse_dcc_message(const uint8_t* data, size_t len);
  void set_decoder_address(uint16_t address);
  void handle_dcc_message(const DCCMessage& msg);
  const std::vector<uint16_t>& get_discovered_addresses() const;


private:
  void send_discovery_response();

  uart_inst_t* _uart;
  uint _txPin;
  uint _rxPin;
  uint16_t _decoder_address;
  std::vector<uint16_t> _discovered_addresses;
};

#endif
