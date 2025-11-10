/**
 * @file RailcomProtocolDefs.h
 * @brief Defines constants and magic numbers used throughout the RailCom library.
 * @details This file centralizes various protocol-specific values, such as timings,
 *          baud rates, and address range limits, as defined in the RCN-217 and
 *          RCN-218 specifications. This makes the code more readable and easier
 *          to maintain.
 */
#ifndef RAILCOM_PROTOCOL_DEFS_H
#define RAILCOM_PROTOCOL_DEFS_H

/** @name PIO Configuration */
///@{
/** @brief The pulse width for the DCC cutout signal in PIO clock cycles. @see RCN-217 */
constexpr uint32_t PIO_CUTOUT_PULSE_WIDTH = 1772;
///@}

/** @name UART Configuration */
///@{
/** @brief The standard baud rate for DCC communication. */
constexpr uint32_t UART_DCC_BAUDRATE = 115200;
/** @brief The standard baud rate for RailCom communication. @see RCN-217, 3.2 */
constexpr uint32_t UART_RAILCOM_BAUDRATE = 250000;
///@}

/** @name Timing */
///@{
/** @brief The mandatory delay between the end of Channel 1 and the start of Channel 2. @see RCN-217, 4.2 */
constexpr uint32_t RAILCOM_CH2_DELAY_US = 193;
///@}

/** @name Address Ranges */
///@{
/** @brief The minimum valid short address for a DCC decoder. */
constexpr uint16_t MIN_SHORT_ADDRESS = 1;
/** @brief The maximum valid short address for a DCC decoder. */
constexpr uint16_t MAX_SHORT_ADDRESS = 127;
/** @brief The maximum valid address for an accessory decoder. */
constexpr uint16_t MAX_ACCESSORY_ADDRESS = 2047;
///@}

/** @name XPOM */
///@{
/** @brief The maximum sequence number for an Extended POM (XPOM) message. @see RCN-217, 5.2.9 */
constexpr uint8_t MAX_XPOM_SEQUENCE = 3;
///@}

/** @name Rerailing */
///@{
/** @brief The maximum value for the seconds counter in a rerailing message. @see RCN-217, 5.2.12 */
constexpr uint8_t MAX_RERAIL_SECONDS = 255;
///@}

/** @name Data Space */
///@{
/** @brief The bitmask to extract the length from the header of a Data Space message. @see RCN-218, 5.2 */
constexpr uint8_t DATASPACE_LEN_MASK = 0x1F;
///@}

#endif // RAILCOM_PROTOCOL_DEFS_H
