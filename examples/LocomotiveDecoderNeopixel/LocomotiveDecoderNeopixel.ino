#include <Arduino.h>
#include <NmraDcc.h>
#include <Adafruit_NeoPixel.h>
#include <map>
#include "RailcomTx.h"
#include "RailcomTxManager.h"
#include "DecoderStateMachine.h"

const uint16_t LOCOMOTIVE_ADDRESS = 4098;

#define NEOPIXEL_PIN 10
#define NEOPIXEL_COUNT 1

Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint8_t fn_0_4_state = 0;
uint8_t fn_5_8_state = 0;
uint8_t fn_9_12_state = 0;
uint8_t fn_13_20_state = 0;
uint8_t fn_21_28_state = 0;

std::map<uint8_t, uint32_t> functionColors = {
    {4, pixels.Color(255, 165, 0)}, // Orange
    {5, pixels.Color(255, 255, 0)}, // Yellow
    {6, pixels.Color(75, 0, 130)},   // Indigo
    {7, pixels.Color(148, 0, 211)},  // Violet
};

RailcomTx sender(uart0, 0, 1);
RailcomTxManager txManager(sender);
DecoderStateMachine stateMachine(txManager, DecoderType::LOCOMOTIVE, LOCOMOTIVE_ADDRESS);
NmraDcc dcc;

void notifyDccSpeedPacket(uint16_t address, DCC_ADDR_TYPE addr_type, uint8_t speed, DCC_DIRECTION forward) {
    // We can get the raw packet from the NmraDcc library
    DCCMessage dcc_msg(dcc.getPacket(), dcc.getPacketSize());

    // Let the state machine handle the logic
    stateMachine.handleDccPacket(dcc_msg);

    // Set the brightness based on the speed
    pixels.setBrightness(speed > 0 ? map(speed, 1, 127, 0, 255) : 0);
    pixels.show();

    // Trigger the cutout so the message can be sent
    sender.send_dcc_with_cutout(dcc_msg);
}

void notifyDccFunc(uint16_t address, DCC_ADDR_TYPE addr_type, FN_GROUP func_group, uint8_t func_state) {
    switch (func_group) {
        case FN_0_4:
            fn_0_4_state = func_state;
            break;
        case FN_5_8:
            fn_5_8_state = func_state;
            break;
        case FN_9_12:
            fn_9_12_state = func_state;
            break;
        case FN_13_20:
            fn_13_20_state = func_state;
            break;
        case FN_21_28:
            fn_21_28_state = func_state;
            break;
        default:
            break;
    }

    if (fn_0_4_state == 0 && fn_5_8_state == 0 && fn_9_12_state == 0 && fn_13_20_state == 0 && fn_21_28_state == 0) {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    } else if (fn_0_4_state & FN_BIT_00) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    } else if (fn_0_4_state & FN_BIT_01) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    } else if (fn_0_4_state & FN_BIT_02) {
        pixels.setPixelColor(0, pixels.Color(0, 255, 0));
    } else if (fn_0_4_state & FN_BIT_03) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 255));
    } else {
        for (auto const& [key, val] : functionColors) {
            if (key == 4 && (fn_0_4_state & FN_BIT_04)) {
                pixels.setPixelColor(0, val);
                pixels.show();
                return;
            } else if (key >= 5 && key <= 8 && (fn_5_8_state & (1 << (key - 5)))) {
                pixels.setPixelColor(0, val);
                pixels.show();
                return;
            } else if (key >= 9 && key <= 12 && (fn_9_12_state & (1 << (key - 9)))) {
                pixels.setPixelColor(0, val);
                pixels.show();
                return;
            } else if (key >= 13 && key <= 20 && (fn_13_20_state & (1 << (key - 13)))) {
                pixels.setPixelColor(0, val);
                pixels.show();
                return;
            } else if (key >= 21 && key <= 28 && (fn_21_28_state & (1 << (key - 21)))) {
                pixels.setPixelColor(0, val);
                pixels.show();
                return;
            }
        }
    }
    pixels.show();
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    dcc.begin(INPUT_PIN, 0);
    dcc.setSpeedPacketHandler(notifyDccSpeedPacket, true);
    dcc.setFunctionPacketHandler(notifyDccFunc, true);
    sender.begin();

    pixels.begin();
    pixels.setBrightness(0);
    pixels.show();

    Serial.println("Locomotive Decoder (NMRA) with Neopixel Example");
}

void loop() {
    dcc.process();
    sender.task();
}
