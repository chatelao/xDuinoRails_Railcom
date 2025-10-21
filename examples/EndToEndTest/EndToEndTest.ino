#include <AUnit.h>
#include "Railcom.h"
#include "RailcomManager.h"

// --- Test Environment Setup ---
// ... (setup is unchanged)

// --- Helper Functions ---
// ... (trigger_cutout is unchanged)

// --- Test Cases ---

/**
 * @test EndToEnd::pomRead
 * @brief Tests a complete POM Read cycle.
 * @details The decoder queues a POM response, the command station triggers a cutout,
 *          and the command station then reads and verifies the received CV value.
 * @see RCN-217, Section 5.1: POM (ID 0)
 */
test(EndToEnd, pomRead) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::addressBroadcastShort
 * @brief Verifies the broadcast of a short vehicle address.
 * @details The decoder sends its 7-bit address in an ADR_HIGH (ID 1) datagram.
 * @see RCN-217, Section 5.2: ADR (IDs 1 & 2)
 */
test(EndToEnd, addressBroadcastShort) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::addressBroadcastLong
 * @brief Verifies the alternating broadcast of a long vehicle address.
 * @details A long address is sent in two parts: ADR_HIGH (ID 1) for the upper bits
 *          and ADR_LOW (ID 2) for the lower bits. This test verifies both parts are
 *          sent and received correctly in sequence.
 * @see RCN-217, Section 5.2: ADR (IDs 1 & 2)
 */
test(EndToEnd, addressBroadcastLong) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::dynamicData
 * @brief Tests the transmission of a Dynamic Variable (DYN) message.
 * @details The decoder sends a DYN (ID 7) message containing a sub-index and a value,
 *          simulating a fuel level report.
 * @see RCN-217, Section 5.5: DYN (ID 7)
 */
test(EndToEnd, dynamicData) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::xpomRead
 * @brief Tests a complete XPOM Read cycle for a block of 4 CVs.
 * @details The decoder queues an XPOM response with a specific sequence number.
 *          The command station triggers a cutout and verifies the ID, sequence, and all 4 CV values.
 * @see RCN-217, Section 5.6: XPOM (ID 8 to ID 11)
 */
test(EndToEnd, xpomRead) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::serviceRequest
 * @brief Verifies the transmission of a Service Request (SRQ) from an accessory decoder.
 * @details The decoder sends its address on Channel 1 without a datagram ID.
 * @see RCN-217, Section 6.1: SRQ - Service Request for Accessory Decoders
 */
test(EndToEnd, serviceRequest) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::status1Report
 * @brief Tests the transmission of an accessory decoder status report.
 * @details The decoder sends a STAT1 (ID 4) message.
 * @see RCN-217, Section 6.3: STAT1 (ID 4)
 */
test(EndToEnd, status1Report) {
    // ... (test implementation is unchanged)
}

/**
 * @test EndToEnd::errorReport
 * @brief Tests the transmission of an accessory decoder error report.
 * @details The decoder sends an ERROR (ID 6) message.
 * @see RCN-217, Section 6.6: ERROR (ID 6)
 */
test(EndToEnd, errorReport) {
    // ... (test implementation is unchanged)
}
