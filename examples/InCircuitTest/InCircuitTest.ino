#include <Arduino.h>
#include <Railcom.h>

// Sender on UART0
RailcomSender sender(uart0, 0, 1);

// Receiver on UART1
RailcomReceiver receiver(uart1, 4, 5);

void setup() {
  Serial.begin(115200);
  while (!Serial); // wait for serial port to connect. Needed for native USB

  sender.begin();
  receiver.begin();
  receiver.set_decoder_address(0x1234);

  // Send a discovery request
  uint8_t data[] = {0xFF, 0xFF, 0x01};
  DCCMessage msg(data, sizeof(data));
  sender.send_dcc_async(msg);

  // Wait for the response
  delay(10);

  // Read the response
  uint8_t buffer[2];
  if (receiver.read_response(buffer, sizeof(buffer), 100)) {
    uint16_t address = (buffer[0] << 8) | buffer[1];
    if (address == 0x1234) {
      Serial.println("In-circuit test passed!");
    } else {
      Serial.print("In-circuit test failed: incorrect address. Expected 0x1234, got 0x");
      Serial.println(address, HEX);
    }
  } else {
    Serial.println("In-circuit test failed: no response");
  }
}

void loop() {
}
