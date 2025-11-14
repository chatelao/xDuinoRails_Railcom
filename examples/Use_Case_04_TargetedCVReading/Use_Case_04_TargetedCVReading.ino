/**
 * @file Use_Case_04_TargetedCVReading.ino
 * @brief Demonstrates how to read a CV value from a specific locomotive.
 * @details This example simulates a command station requesting the value of
 *          CV 8 (Manufacturer ID) from a locomotive with address 100.
 *          It uses the CommandStationRx class to wait for the locomotive to be
 *          identified and then to catch the subsequent POM response.
 */

#include <Arduino.h>
#include "RP2040RailcomRxHardware.h"
#include "RailcomRx.h"
#include "CommandStationRx.h"

// --- Hardware Configuration ---
#define RAILCOM_RX_PIN 5 // UART1 RX on GP5

// --- Global Objects ---
RP2040RailcomRxHardware railcom_hw(uart1, RAILCOM_RX_PIN);
RailcomRx railcom_rx(&railcom_hw);
CommandStationRx command_station(&railcom_rx);

// --- State Management for our Example ---
enum class AppState {
  WAITING_FOR_REQUEST,
  WAITING_FOR_LOCO,
  WAITING_FOR_POM,
  DONE
};

AppState currentState = AppState::WAITING_FOR_REQUEST;
uint16_t targetAddress = 100;

/**
 * @brief Called when any locomotive is identified.
 * @details We check if the identified address is the one we are waiting for.
 */
void handleLocoIdentified(uint16_t address) {
  Serial.print("Detected locomotive with address: ");
  Serial.println(address);
  if (currentState == AppState::WAITING_FOR_LOCO && address == targetAddress) {
    Serial.println("Target locomotive found! Now waiting for POM response...");
    currentState = AppState::WAITING_FOR_POM;
  }
}

/**
 * @brief Called when a POM response is received.
 * @details We check if it came from our target locomotive.
 */
void handlePomResponse(uint16_t sourceAddress, uint8_t cvValue) {
  if (currentState == AppState::WAITING_FOR_POM && sourceAddress == targetAddress) {
    Serial.print("SUCCESS! Received POM response from address ");
    Serial.print(sourceAddress);
    Serial.print(": CV8 = ");
    Serial.println(cvValue);
    currentState = AppState::DONE;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println("--- RailCom Targeted CV Reading Example ---");

  railcom_rx.begin();
  command_station.begin();

  // Attach both callbacks
  command_station.onLocoIdentified(handleLocoIdentified);
  command_station.onPomResponse(handlePomResponse);

  Serial.print("Type 'read' to simulate sending a DCC POM Read request for CV8 to address ");
  Serial.println(targetAddress);
}

void loop() {
  // Process incoming RailCom messages
  command_station.task();

  // Simple state machine triggered by serial input
  if (currentState == AppState::WAITING_FOR_REQUEST) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      if (cmd == "read") {
        Serial.println("Simulating DCC request: 'Read CV8 from address 100'");
        Serial.println("Place the locomotive on the track now...");
        currentState = AppState::WAITING_FOR_LOCO;
      }
    }
  }

  if (currentState == AppState::DONE) {
    Serial.println("Task complete. Restarting...");
    delay(5000);
    currentState = AppState::WAITING_FOR_REQUEST;
     Serial.print("Type 'read' to simulate sending a DCC POM Read request for CV8 to address ");
    Serial.println(targetAddress);
  }
}
