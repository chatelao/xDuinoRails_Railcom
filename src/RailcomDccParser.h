#ifndef RAILCOM_DCC_PARSER_H
#define RAILCOM_DCC_PARSER_H

#include "Railcom.h"
#include <functional>

class RailcomDccParser {
public:
    // Callbacks for RCN-218 commands
    std::function<void(uint8_t, uint16_t, uint8_t)> onLogonEnable;
    std::function<void(uint16_t, uint32_t, uint8_t, const uint8_t*, size_t)> onSelect;
    std::function<void(uint16_t, uint32_t, uint16_t)> onLogonAssign;
    std::function<void()> onGetDataStart;
    std::function<void()> onGetDataCont;
    std::function<void(const uint8_t*, size_t)> onSetData;
    std::function<void()> onSetDataEnd;

    // Callbacks for RCN-217 commands

    // Called when a POM Read CV command is received.
    // @param cv The CV to read.
    // @param address The address of the decoder.
    std::function<void(uint16_t cv, uint16_t address)> onPomReadCv;

    // Called when a POM Write CV command is received.
    // @param cv The CV to write.
    // @param value The value to write.
    // @param address The address of the decoder.
    std::function<void(uint16_t cv, uint8_t value, uint16_t address)> onPomWriteCv;

    // Called when a POM Write Bit command is received.
    // @param cv The CV to write.
    // @param bit The bit to write.
    // @param value The value of the bit.
    // @param address The address of the decoder.
    std::function<void(uint16_t cv, uint8_t bit, uint8_t value, uint16_t address)> onPomWriteBit;

    // Called when an accessory command is received.
    // @param address The address of the accessory.
    // @param activate Whether to activate or deactivate the accessory.
    // @param output The output of the accessory to control.
    std::function<void(uint16_t address, bool activate, uint8_t output)> onAccessory;

    // Called when a function command is received.
    // @param address The address of the decoder.
    // @param function The function number.
    // @param state The state of the function.
    std::function<void(uint16_t address, uint8_t function, bool state)> onFunction;

    void parse(const DCCMessage& msg, bool* response_sent = nullptr);
};

#endif // RAILCOM_DCC_PARSER_H
