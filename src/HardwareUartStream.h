#ifndef HARDWARE_UART_STREAM_H
#define HARDWARE_UART_STREAM_H

#include "IRailcomStream.h"
#include "hardware/uart.h"

class HardwareUartStream : public IRailcomStream {
public:
    HardwareUartStream(uart_inst_t* uart, uint txPin, uint rxPin);

    void begin(uint32_t baudrate) override;
    void end() override;
    int available() override;
    int read() override;
    void write(const uint8_t* data, size_t len) override;
    void setBaudrate(uint32_t baudrate) override;
    void tx_wait_blocking() override;

private:
    uart_inst_t* _uart;
    uint _txPin;
    uint _rxPin;
};

#endif // HARDWARE_UART_STREAM_H
