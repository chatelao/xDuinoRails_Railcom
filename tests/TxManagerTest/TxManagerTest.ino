#include <AUnit.h>
#include "RailcomTx.h"
#include "RailcomTxManager.h"

// Mock Sender to inspect the queue
class TestableRailcomTx : public RailcomTx {
public:
    TestableRailcomTx() : RailcomTx(nullptr, 0, 0) {}
    // We make the queues public for testing via the preprocessor directive in RailcomTx.h
};

test(TxManager, queuePomResponse) {
    TestableRailcomTx sender;
    RailcomTxManager txManager(sender);

    txManager.sendPomResponse(123);

    assertFalse(sender._ch2_queue.empty());
    std::vector<uint8_t> msg = sender._ch2_queue.front();

    // Expected: ID 0 (POM), 8-bit payload 123 -> 12 bits
    assertEqual(msg.size(), 2);
    assertEqual(msg[0], 0b10101010); // Encoded chunk 1 (000001)
    assertEqual(msg[1], 0b00110101); // Encoded chunk 2 (111011)
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    TestRunner::run();
}

void loop() {}
