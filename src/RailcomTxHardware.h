/**
 * @file RailcomTxHardware.h
 * @brief Defines the abstract hardware interface for RailCom transmission.
 */
#ifndef RAILCOM_TX_HARDWARE_H
#define RAILCOM_TX_HARDWARE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "Railcom.h"

/**
 * @class RailcomTxHardware
 * @brief An abstract base class for the RailCom transmitter hardware abstraction layer (HAL).
 */
class RailcomTxHardware {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~RailcomTxHardware() = default;

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
     * @brief Sends a vector of raw, 4-of-8 encoded RailCom bytes over the hardware interface.
     * @param bytes The raw bytes to be sent. These are expected to be already encoded.
     */
    virtual void send_bytes(const std::vector<uint8_t>& bytes) = 0;
};

#endif // RAILCOM_TX_HARDWARE_H
