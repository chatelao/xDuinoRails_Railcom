/**
 * AccessoryDecoderFeedback.ino
 *
 * This sketch demonstrates an advanced accessory decoder that provides feedback
 * using various RailCom messages. It simulates the operation of a turnout
 * and reports its status back to the command station.
 *
 * Use Case Coverage:
 * - #10: Verifying Turnout Position (STAT4)
 * - #11: Reporting a Jammed Turnout (ERROR)
 * - #12: Displaying Turnout Switching Time (TIME)
 */

#include <Arduino.h>
#include <NmraDcc.h>
#include <Railcom.h>
#include <RailcomTx.h>
#include <RP2040RailcomTxHardware.h>

// --- Configuration ---
#define DCC_PIN 2
#define DECODER_ADDRESS 100 // The base address for this decoder.

// --- Global Objects ---
NmraDcc Dcc;
// Correctly initialize the hardware with the UART and TX pin
RP2040RailcomTxHardware railcom_hardware(uart1, 4); // Use UART1 on GP4
RailcomTx railcom_tx(&railcom_hardware);

// --- Turnout State Machine ---
enum TurnoutState { IDLE, MOVING, JAMMED };
TurnoutState turnout_state = IDLE;
unsigned long movement_start_time = 0;
const unsigned long MOVEMENT_DURATION_MS = 2000; // 2 seconds
uint8_t current_position = 0; // 0 = thrown, 1 = closed

// Callback function to handle DCC accessory packets
// This function name is automatically registered by NmraDcc when CV29_ACCESSORY_DECODER is set
void notifyDccAccTurnoutOutput(uint16_t address, uint8_t direction, uint8_t power) {
    // We handle a block of 4 addresses, but only care about the base address.
    if (address != DECODER_ADDRESS) {
        return;
    }

    // Check if we are busy
    if (turnout_state != IDLE) {
        return;
    }

    // Simulate a jam every 5th command for demonstration
    static int command_count = 0;
    if (++command_count % 5 == 0) {
        turnout_state = JAMMED;
        Serial.println("Simulating a JAMMED turnout!");
        // Report the error on Channel 2. Error code 3 = Accessory Malfunction
        railcom_tx.sendError(3);
    } else {
        turnout_state = MOVING;
        movement_start_time = millis();
        current_position = direction;
        Serial.print("Turnout moving to position: ");
        Serial.println(current_position);
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Accessory Decoder with Feedback starting...");

    // Initialize DCC listener
    Dcc.pin(DCC_PIN, false, true);
    // Use the correct CV29 constant
    Dcc.init(MAN_ID_DIY, 10, CV29_ACCESSORY_DECODER | CV29_RAILCOM_ENABLE, 0);

    // Initialize RailCom Transmitter
    railcom_tx.begin();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    // Process incoming DCC packets
    Dcc.process();

    // The new NmraDcc library doesn't have a cutout detection.
    // For a real decoder, you would detect the absence of a DCC signal.
    // For this example, we'll just periodically check our state machine
    // and send messages. A better approach might be to trigger this
    // right after Dcc.process() if a packet was received.
    // To simulate, we just call on_cutout_start frequently.
    railcom_tx.on_cutout_start();

    // --- Handle Turnout State Machine ---
    if (turnout_state == MOVING) {
        unsigned long elapsed = millis() - movement_start_time;
        if (elapsed < MOVEMENT_DURATION_MS) {
            // Still moving, report remaining time.
            uint8_t time_remaining_deci_seconds = (MOVEMENT_DURATION_MS - elapsed) / 100;
            // Send time with unit flag (false for 0.1s)
            railcom_tx.sendTime(time_remaining_deci_seconds, false);
        } else {
            // Movement finished
            turnout_state = IDLE;
            Serial.println("Movement complete.");
            // Report the final status.
            uint8_t status_byte = (current_position == 1) ? 0b00000010 : 0b00000001;
            railcom_tx.sendStatus4(status_byte);
        }
    } else if (turnout_state == JAMMED) {
        // After sending the error, we reset to IDLE.
        // In a real decoder, this might require manual intervention.
        turnout_state = IDLE;
    }
}
