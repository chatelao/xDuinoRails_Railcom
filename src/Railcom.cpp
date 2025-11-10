/**
 * @file Railcom.cpp
 * @brief Implementation of the DCCMessage class.
 */
#include "Railcom.h"
#include <cstring>
#include <vector>

// --- DCCMessage Implementation ---

/**
 * @brief Default constructor for DCCMessage. Initializes an empty message.
 */
DCCMessage::DCCMessage() : _len(0) {}

/**
 * @brief Constructs a DCCMessage from a raw byte array.
 * @details Copies the provided data into the internal buffer. The length is
 *          clamped to the maximum size of the internal buffer to prevent overflows.
 * @param data Pointer to the raw DCC packet data.
 * @param len The length of the data array.
 */
DCCMessage::DCCMessage(const uint8_t* data, size_t len) : _len(len > sizeof(_data) ? sizeof(_data) : len) {
    memcpy(_data, data, _len);
}

/**
 * @brief Gets a constant pointer to the internal data buffer.
 * @return A const uint8_t* pointer to the message data.
 */
const uint8_t* DCCMessage::getData() const { return _data; }

/**
 * @brief Gets the length of the DCC message.
 * @return The length of the message in bytes.
 */
size_t DCCMessage::getLength() const { return _len; }
