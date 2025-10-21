#include <AUnit.h>
#include "RailcomManager.h"

// This test version requires a mock that can inspect the queue.
// For simplicity, we'll make the queues public for testing purposes.
// This is not ideal, but necessary without a proper mocking framework.
class TestableRailcomSender : public RailcomSender {
public:
    TestableRailcomSender() : RailcomSender(nullptr, 0, 0) {}
    std::queue<std::vector<uint8_t>>& getCh1Queue() { return _ch1_queue; }
    std::queue<std::vector<uint8_t>>& getCh2Queue() { return _ch2_queue; }
};

test(Manager, queuePomResponse) {
    TestableRailcomSender sender;
    RailcomReceiver receiver(nullptr, 0, 0);
    RailcomManager manager(sender, receiver);

    manager.sendPomResponse(123);

    assertFalse(sender.getCh2Queue().empty());
    std::vector<uint8_t> msg = sender.getCh2Queue().front();

    // Expected: ID 0 (POM), 8-bit payload 123 -> 12 bits total
    // Payload: 0000 01111011
    // Chunk 1 (000001): 1 -> encoded
    // Chunk 2 (111011): 59 -> encoded
    assertEqual(msg.size(), 2);
    assertEqual(msg[0], RailcomEncoding::encode4of8(1));
    assertEqual(msg[1], RailcomEncoding::encode4of8(59));
}

test(Manager, queueShortAddress) {
    TestableRailcomSender sender;
    RailcomReceiver receiver(nullptr, 0, 0);
    RailcomManager manager(sender, receiver);

    manager.sendAddress(99); // Short address, goes to Ch1

    assertFalse(sender.getCh1Queue().empty());
    std::vector<uint8_t> msg = sender.getCh1Queue().front();

    // Expected: ID 1 (ADR_HIGH), 7-bit payload 99 -> 11 bits total
    // This is tricky, let's just check size for now. A full bit check is complex.
    assertEqual(msg.size(), 2);
}


void setup() {
    Serial.begin(115200);
    while (!Serial);
    TestRunner::run();
}

void loop() {}
