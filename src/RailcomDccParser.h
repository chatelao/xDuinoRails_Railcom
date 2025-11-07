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
    std::function<void(uint16_t cv, uint16_t address)> onPomReadCv;
    std::function<void(uint16_t cv, uint8_t value, uint16_t address)> onPomWriteCv;
    std::function<void(uint16_t cv, uint8_t bit, uint8_t value, uint16_t address)> onPomWriteBit;

    std::function<void(uint16_t address, bool activate, uint8_t output)> onAccessory;
    std::function<void(uint8_t function, bool state)> onFunction;

    void parse(const DCCMessage& msg);
};

#endif // RAILCOM_DCC_PARSER_H
