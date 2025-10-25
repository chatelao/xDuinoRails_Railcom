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
    bool available();
    RailcomMessage getMessage();

private:
    enum class State { IDLE, READING };
    State _state;
    std::vector<uint8_t> _buffer;
    RailcomHardware* _hardware;
};

#endif // RAILCOM_RX_H
