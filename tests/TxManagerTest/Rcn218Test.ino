#include <ArduinoUnit.h>
#include "RailcomTx.h"

// Mock RailcomTx to capture queued messages
class MockRailcomTx : public RailcomTx {
public:
    MockRailcomTx() : RailcomTx(nullptr, 0, 0) {}

    void queue_message(uint8_t channel, const std::vector<uint8_t>& data) {
        last_channel = channel;
        last_data = data;
    }

    uint8_t last_channel;
    std::vector<uint8_t> last_data;
};

test(Rcn218_SendDecoderUnique) {
    MockRailcomTx mockTx;

    mockTx.sendDecoderUnique(0x0123, 0x456789AB);

    // Expected encoded data for DECODER_UNIQUE
    // ID=15 (0xF), Manuf=0x0123, Prod=0x456789AB
    // Payload: 0xF0123456789AB
    // This needs to be 4of8 encoded.
    // For simplicity, we'll just check the first byte.
    // First 6 bits: 111100 (0x3C) -> encoded: 0b10011100
    assertEqual(mockTx.last_data[0], 0b10011100);
}

test(Rcn218_SendDecoderState) {
    MockRailcomTx mockTx;

    mockTx.sendDecoderState(0xAB, 0x1234, 0x5678);

    // Expected encoded data for DECODER_STATE
    // ID=13 (0xD), Flags=0xAB, Count=0x1234, Caps=0x5678
    // First 6 bits: 110110 (0x36) -> encoded: 0b01101001
    assertEqual(mockTx.last_data[0], 0b01101001);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Test::run();
}

void loop() {
}
