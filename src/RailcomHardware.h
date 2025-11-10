/**
 * @file RailcomHardware.h
 * @brief Defines the abstract hardware interface for RailCom communication.
 * @details This file specifies the pure virtual functions that any hardware-specific
 *          implementation must provide to be compatible with the RailcomTx and
 *          RailcomRx classes. This abstraction allows the core RailCom logic to
 *          be independent of the underlying microcontroller or communication hardware.
 */
#ifndef RAILCOM_HARDWARE_H
#define RAILCOM_HARDWARE_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include "Railcom.h"

/**
 * @class RailcomHardware
 * @brief An abstract base class for the RailCom hardware abstraction layer (HAL).
 * @details This class defines the standard interface for sending and receiving
 *          raw RailCom bytes. Concrete implementations of this class will handle
 *          the specifics of a particular hardware platform, such as UART, PIO, etc.
 */
class RailcomHardware {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~RailcomHardware() = default;

    /**
     * @brief Initializes the hardware for RailCom communication.
     * @details This should configure the necessary pins, peripherals (e.g., UART),
     *          and any other hardware resources required.
     */
    virtual void begin() = 0;

    /**
     * @brief Deinitializes and releases hardware resources.
     */
    virtual void end() = 0;

    /**
     * @brief A periodic task function for the hardware layer.
     * @details This can be used for tasks that require regular polling, such as
     *          checking DMA buffers or managing state machines within the hardware driver.
     */
    virtual void task() = 0;

    /**
     * @brief Sends a vector of raw, 4-of-8 encoded RailCom bytes over the hardware interface.
     * @param bytes The raw bytes to be sent. These are expected to be already encoded.
     */
    virtual void send_bytes(const std::vector<uint8_t>& bytes) = 0;

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

#endif // RAILCOM_HARDWARE_H
