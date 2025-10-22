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

    void parse(const DCCMessage& msg);
};

#endif // RAILCOM_DCC_PARSER_H
