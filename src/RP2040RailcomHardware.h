/**
 * @file RP2040RailcomHardware.h
 * @brief A concrete implementation of the RailcomHardware interface for the Raspberry Pi RP2040.
 * @details This class uses one of the RP2040's hardware UART peripherals to handle
 *          the physical layer of RailCom communication, sending and receiving bytes
 *          at the specified 250,000 baud rate.
 */
#ifndef RP2040_RAILCOM_HARDWARE_H
#define RP2040_RAILCOM_HARDWARE_H

#include "RailcomHardware.h"
#include "hardware/uart.h"
#include <vector>

/**
 * @class RP2040RailcomHardware
 * @brief Implements the RailcomHardware interface using an RP2040 UART.
 */
class RP2040RailcomHardware : public RailcomHardware {
public:
    /**
     * @brief Constructs an RP2040RailcomHardware object.
     * @param uart A pointer to the RP2040 UART instance to use (e.g., `uart0`, `uart1`).
     * @param tx_pin The GPIO pin number to use for UART TX.
     * @param rx_pin The GPIO pin number to use for UART RX.
     */
    RP2040RailcomHardware(uart_inst_t* uart, uint tx_pin, uint rx_pin);

    /**
     * @brief Default destructor.
     */
    ~RP2040RailcomHardware() override = default;

    /**
     * @brief Initializes the specified UART peripheral for RailCom.
     * @see RailcomHardware::begin()
     */
    void begin() override;

    /**
     * @brief Deinitializes the UART peripheral.
     * @see RailcomHardware::end()
     */
    void end() override;

    /**
     * @brief Periodic task function (currently unused for this implementation).
     * @see RailcomHardware::task()
     */
    void task() override;

    /**
     * @brief Sends a vector of bytes over the UART.
     * @see RailcomHardware::send_bytes()
     */
    void send_bytes(const std::vector<uint8_t>& bytes) override;

    /**
     * @brief Checks if data is available on the UART FIFO.
     * @see RailcomHardware::available()
     */
    int available() override;

    /**
     * @brief Reads a single byte from the UART FIFO.
     * @see RailcomHardware::read()
     */
    int read() override;

private:
    uart_inst_t* _uart; ///< Pointer to the RP2040 UART instance.
    uint _tx_pin;       ///< The GPIO pin for UART TX.
    uint _rx_pin;       ///< The GPIO pin for UART RX.
};

#endif // RP2040_RAILCOM_HARDWARE_H
