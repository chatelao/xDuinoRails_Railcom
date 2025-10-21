#include <Arduino.h>
#include "Railcom.h"

Railcom railcom1(uart0, 0, 1);
Railcom railcom2(uart1, 2, 3);

void setup() {
  Serial.begin(115200);
  railcom1.begin();
  railcom2.begin();
}

void loop() {
  // Send a discovery request from railcom1
  uint8_t data[] = {0xFF, 0xFF, 0x01};
    DCCMessage msg(data, sizeof(data));
  railcom1.send_dcc_async(msg);

  // Read the response on railcom2
  uint8_t buffer[2];
  if (railcom2.read_response(buffer, sizeof(buffer), 100)) {
    Serial.print("Received response: ");
    Serial.print(buffer[0], HEX);
    Serial.print(" ");
    Serial.println(buffer[1], HEX);
  } else {
    Serial.println("No response");
  }
  delay(1000);
}
