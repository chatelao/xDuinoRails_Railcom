#include <Arduino.h>
#include <Wire.h>
#include <NmraDcc.h>
#include "Railcom.h"
#include "RailcomReceiver.h"
#include "RailcomRxManager.h"
#include "SerialPIO.h"

class PioRailcomReceiver : public RailcomReceiver {
public:
    PioRailcomReceiver(pin_size_t rx_pin) : RailcomReceiver(nullptr, 0), _pio(rx_pin, -1) {}
    void begin() override {
        _pio.begin(250000);
    }
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms) override {
        unsigned long start = millis();
        while (millis() - start < timeout_ms) {
            if (_pio.available()) {
                buffer.push_back(_pio.read());
            }
        }
        return buffer.size() > 0;
    }
private:
    SerialPIO _pio;
};

// I2C Address for this device
#define I2C_ADDRESS 0x08

#if defined(ARDUINO_RASPBERRY_PI_PICO)
#define NUM_CHANNELS 10
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
#define NUM_CHANNELS 10
#else
#define NUM_CHANNELS 14 // Assume RP2350
#endif

// DCC input pin
#define INPUT_PIN 6

// Railcom receivers
RailcomReceiver* receivers[NUM_CHANNELS];
RailcomRxManager* rxManagers[NUM_CHANNELS];

// I2C Data Structure
// We use a fixed-size struct to send the Railcom data over I2C.
// The struct is large enough to hold the largest possible Railcom message.
struct I2cRailcomData {
    uint8_t size;
    uint8_t data[8];
    DCCMessage dcc_msg;
};

// Data buffers for the latest Railcom messages
I2cRailcomData lastMsgs[NUM_CHANNELS];

// NmraDcc parser
NmraDcc dcc;

// Function prototypes
void onI2CRequest();
void notifyDccPacket(uint16_t address, DCC_ADDR_TYPE addr_type, uint8_t data[], uint8_t len);

#include "SerialPIO.h"

void setup() {
    // Start Serial for debugging
    Serial.begin(115200);
    while (!Serial);
    Serial.println("I2C Railcom Hub");

    // Initialize I2C
    Wire.begin(I2C_ADDRESS);
    Wire.onRequest(onI2CRequest);

    // Initialize Railcom receivers
    // TODO: Customize the pins for your setup
    receivers[0] = new RailcomReceiver(uart0, 2);
    rxManagers[0] = new RailcomRxManager(*receivers[0]);
    receivers[0]->begin();
    receivers[1] = new RailcomReceiver(uart1, 4);
    rxManagers[1] = new RailcomRxManager(*receivers[1]);
    receivers[1]->begin();

    for (int i = 2; i < NUM_CHANNELS; i++) {
        receivers[i] = new PioRailcomReceiver(i + 7); // Use pins 9, 10, ...
        rxManagers[i] = new RailcomRxManager(*receivers[i]);
        receivers[i]->begin();
    }

    // Initialize NmraDcc
    dcc.begin(INPUT_PIN, 0);
    dcc.setPacketHandler(notifyDccPacket, true);
}

void loop() {
    // Process DCC packets
    dcc.process();
}

void onI2CRequest() {
    // Send the latest Railcom data to the master
    for (int i = 0; i < NUM_CHANNELS; i++) {
        Wire.write((uint8_t*)&lastMsgs[i], sizeof(I2cRailcomData));
    }
}

void notifyDccPacket(uint16_t address, DCC_ADDR_TYPE addr_type, uint8_t data[], uint8_t len) {
    // This function is called by the NmraDcc library when a DCC packet is received.
    // We store the DCC message so we can send it over I2C with the Railcom data.
    DCCMessage dcc_msg(data, len);

    // Process Railcom messages
    for (int i = 0; i < NUM_CHANNELS; i++) {
        RailcomMessage* msg = rxManagers[i]->readMessage();
        if (msg != nullptr) {
            lastMsgs[i].dcc_msg = dcc_msg;
            lastMsgs[i].size = sizeof(*msg);
            if (lastMsgs[i].size <= sizeof(lastMsgs[i].data)) {
                memcpy(lastMsgs[i].data, msg, lastMsgs[i].size);
            }
        }
    }
}
