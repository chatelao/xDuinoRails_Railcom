#ifndef RAILCOM_RX_H
#define RAILCOM_RX_H

#include "Railcom.h"
#include "RailcomHardware.h"
#include <vector>

class RailcomRx {
public:
    RailcomRx(RailcomHardware* hardware);
    void begin();
    void task();
    RailcomMessage* read();
    void setContext(DecoderContext context);
    void print(Print& stream);
    void expectDataSpaceResponse(uint8_t dataSpaceNum);

private:
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);
    RailcomMessage* parseMessage(const std::vector<uint8_t>& buffer);
    RailcomHardware* _hardware;
    std::vector<uint8_t> _lastRawBytes;
    RailcomMessage* _lastMessage = nullptr;
    uint8_t _lastAdrHigh = 0;
    DecoderContext _context = DecoderContext::UNKNOWN;
    bool _is_data_space_expected = false;
    uint8_t _expected_data_space_num = 0;
};

#endif // RAILCOM_RX_H
