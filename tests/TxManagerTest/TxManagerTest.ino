#include <ArduinoUnit.h>
#include "RailcomTx.h"
#include "../mocks/MockRailcomTx.h"

test(TxManager_queuePomResponse) {
    MockRailcomTx mockTx;

    mockTx.sendPomResponse(123);

    assertEqual(mockTx.last_channel, 2);
    assertFalse(mockTx.last_data.empty());

    // Expected: ID 0 (POM), 8-bit payload 123 -> 12 bits
    assertEqual(mockTx.last_data.size(), 2);
    assertEqual(mockTx.last_data[0], 0b10101010); // Encoded chunk 1 (000001)
    assertEqual(mockTx.last_data[1], 0b00110101); // Encoded chunk 2 (111011)
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Test::run();
}

void loop() {}
