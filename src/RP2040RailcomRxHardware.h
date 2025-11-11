/**
 * @file RP2040RailcomRxHardware.h
 * @brief A concrete implementation of the RailcomRxHardware interface for the Raspberry Pi RP2040.
 */
#ifndef RP2040_RAILCOM_RX_HARDWARE_H
#define RP2040_RAILCOM_RX_HARDWARE_H

#include "RailcomRxHardware.h"
#include "hardware/uart.h"
#include <vector>

/**
 * @class RP2040RailcomRxHardware
 * @brief Implements the RailcomRxHardware interface using an RP2040 UART.
 */
class RP2040RailcomRxHardware : public RailcomRxHardware {
public:
    /**
     * @brief Constructs an RP2040RailcomRxHardware object.
     * @param uart A pointer to the RP2040 UART instance to use (e.g., `uart0`, `uart1`).
     * @param rx_pin The GPIO pin number to use for UART RX.
     */
    RP2040RailcomRxHardware(uart_inst_t* uart, uint rx_pin);

    /**
     * @brief Default destructor.
     */
    ~RP2040RailcomRxHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;
    int available() override;
    int read() override;

private:
    uart_inst_t* _uart; ///< Pointer to the RP2040 UART instance.
    uint _rx_pin;       ///< The GPIO pin for UART RX.
};

#endif // RP2040_RAILCOM_RX_HARDWARE_H
