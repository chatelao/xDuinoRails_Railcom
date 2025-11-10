#ifndef RAILCOM_RX_HARDWARE_H
#define RAILCOM_RX_HARDWARE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "Railcom.h"

class RailcomRxHardware {
public:
    virtual ~RailcomRxHardware() = default;

    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void task() = 0;

    virtual int available() = 0;
    virtual int read() = 0;
};

#endif // RAILCOM_RX_HARDWARE_H
