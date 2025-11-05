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
    void print(Print& stream);

private:
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);
    RailcomMessage* parseMessage(const std::vector<uint8_t>& buffer);
    RailcomHardware* _hardware;
    std::vector<uint8_t> _lastRawBytes;
    RailcomMessage* _lastMessage = nullptr;
    uint8_t _lastAdrHigh = 0;
};

#endif // RAILCOM_RX_H
