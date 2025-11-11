/**
 * @file RailcomRxHardware.h
 * @brief Defines the abstract hardware interface for RailCom reception.
 */
#ifndef RAILCOM_RX_HARDWARE_H
#define RAILCOM_RX_HARDWARE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "Railcom.h"

/**
 * @class RailcomRxHardware
 * @brief An abstract base class for the RailCom receiver hardware abstraction layer (HAL).
 */
class RailcomRxHardware {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~RailcomRxHardware() = default;

    /**
     * @brief Initializes the hardware for RailCom communication.
     */
    virtual void begin() = 0;

    /**
     * @brief Deinitializes and releases hardware resources.
     */
    virtual void end() = 0;

    /**
     * @brief A periodic task function for the hardware layer.
     */
    virtual void task() = 0;

    /**
     * @brief Checks how many bytes are available to be read from the hardware.
     * @return The number of bytes available in the receive buffer.
     */
    virtual int available() = 0;

    /**
     * @brief Reads a single byte from the hardware's receive buffer.
     * @return The byte that was read, or -1 if no data is available.
     */
    virtual int read() = 0;
};

#endif // RAILCOM_RX_HARDWARE_H
