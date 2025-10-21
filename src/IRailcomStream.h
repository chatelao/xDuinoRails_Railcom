#ifndef I_RAILCOM_STREAM_H
#define I_RAILCOM_STREAM_H

#include <Arduino.h>

// Abstract base class for a serial communication stream.
// This allows the Railcom Sender and Receiver to be decoupled
// from the underlying hardware (e.g., hardware UART vs. PIO SoftSerial).
class IRailcomStream {
public:
    virtual ~IRailcomStream() = default;

    // Initialize the stream with a specific baud rate.
    virtual void begin(uint32_t baudrate) = 0;

    // Change the baud rate of an active stream.
    virtual void setBaudrate(uint32_t baudrate) = 0;

    // Wait for the stream's transmission to complete.
    virtual void tx_wait_blocking() = 0;

    // Stop the stream.
    virtual void end() = 0;

    // Check how many bytes are available to read.
    virtual int available() = 0;

    // Read a byte from the stream. Returns -1 if no data is available.
    virtual int read() = 0;

    // Write a block of data to the stream.
    virtual void write(const uint8_t* data, size_t len) = 0;
};

#endif // I_RAILCOM_STREAM_H
