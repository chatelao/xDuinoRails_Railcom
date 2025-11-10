#ifndef RAILCOM_HARDWARE_H
#define RAILCOM_HARDWARE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "Railcom.h"

class RailcomHardware {
public:
    virtual ~RailcomHardware() = default;

    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void task() = 0;

    virtual void send_bytes(const std::vector<uint8_t>& bytes) = 0;

    virtual int available() = 0;
    virtual int read() = 0;
};

#endif // RAILCOM_HARDWARE_H
