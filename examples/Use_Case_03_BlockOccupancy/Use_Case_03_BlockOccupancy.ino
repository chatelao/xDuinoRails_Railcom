/**
 * @file Use_Case_03_BlockOccupancy.ino
 * @brief Demonstrates a simple block occupancy detector using the CommandStationRx class.
 * @details This example shows how to use the high-level CommandStationRx interface to
 *          easily identify locomotives as they pass through a detection section.
 *          It prints the address of each newly identified locomotive to the Serial Monitor.
 */

#include <Arduino.h>
#include "RP2040RailcomRxHardware.h"
#include "RailcomRx.h"
#include "CommandStationRx.h"

// --- Hardware Configuration ---
// Connect the RailCom detector's output to UART1 RX (GP5 on Raspberry Pi Pico)
#define RAILCOM_RX_PIN 5

// --- Global Objects ---
RP2040RailcomRxHardware railcom_hw(uart1, RAILCOM_RX_PIN);
RailcomRx railcom_rx(&railcom_hw);
CommandStationRx command_station(&railcom_rx);

/**
 * @brief This function is called whenever a complete locomotive address is received.
 * @param address The 14-bit address of the identified locomotive.
 */
void handleLocoIdentified(uint16_t address) {
  Serial.print("Block Occupancy: Locomotive with address ");
  Serial.print(address);
  Serial.println(" detected.");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // Wait for Serial to be ready
  }
  Serial.println("--- RailCom Block Occupancy Detector Example ---");

  // Initialize the RailCom receiver
  railcom_rx.begin();

  // Initialize the command station handler
  command_station.begin();

  // Attach our callback function to the "loco identified" event
  command_station.onLocoIdentified(handleLocoIdentified);

  Serial.println("Initialization complete. Waiting for locomotives...");
}

void loop() {
  // The task() method does all the work of reading and processing messages.
  // It will automatically trigger our callback function when a new address is identified.
  command_station.task();
}
