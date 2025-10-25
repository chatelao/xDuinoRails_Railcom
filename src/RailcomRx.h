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
    RailcomMessage* readMessage();

private:
    bool read_raw_bytes(std::vector<uint8_t>& buffer, uint timeout_ms);
    RailcomMessage* parseMessage(const std::vector<uint8_t>& buffer);
    RailcomHardware* _hardware;
};

#endif // RAILCOM_RX_H
