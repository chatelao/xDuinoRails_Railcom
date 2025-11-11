/**
 * @file RP2040RailcomTxHardware.h
 * @brief A concrete implementation of the RailcomTxHardware interface for the Raspberry Pi RP2040.
 */
#ifndef RP2040_RAILCOM_TX_HARDWARE_H
#define RP2040_RAILCOM_TX_HARDWARE_H

#include "RailcomTxHardware.h"
#include "hardware/uart.h"
#include <vector>

/**
 * @class RP2040RailcomTxHardware
 * @brief Implements the RailcomTxHardware interface using an RP2040 UART.
 */
class RP2040RailcomTxHardware : public RailcomTxHardware {
public:
    /**
     * @brief Constructs an RP2040RailcomTxHardware object.
     * @param uart A pointer to the RP2040 UART instance to use (e.g., `uart0`, `uart1`).
     * @param tx_pin The GPIO pin number to use for UART TX.
     */
    RP2040RailcomTxHardware(uart_inst_t* uart, uint tx_pin);

    /**
     * @brief Default destructor.
     */
    ~RP2040RailcomTxHardware() override = default;

    void begin() override;
    void end() override;
    void task() override;
    void send_bytes(const std::vector<uint8_t>& bytes) override;

private:
    uart_inst_t* _uart; ///< Pointer to the RP2040 UART instance.
    uint _tx_pin;       ///< The GPIO pin for UART TX.
};

#endif // RP2040_RAILCOM_TX_HARDWARE_H
