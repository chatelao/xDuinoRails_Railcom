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

    /**
     * @brief Sends a vector of raw, 4-of-8 encoded RailCom bytes over the hardware interface.
     * @param bytes The raw bytes to be sent. These are expected to be already encoded.
     */
    virtual void send_bytes(const std::vector<uint8_t>& bytes) = 0;

    virtual int available() = 0;
    virtual int read() = 0;
};

#endif // RAILCOM_HARDWARE_H
