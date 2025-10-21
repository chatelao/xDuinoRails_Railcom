#ifndef PIO_SOFT_SERIAL_H
#define PIO_SOFT_SERIAL_H

#include "IRailcomStream.h"
#include "hardware/pio.h"

class PioSoftSerial : public IRailcomStream {
public:
    PioSoftSerial(PIO pio, uint rxPin);

    void begin(uint32_t baudrate) override;
    void end() override;
    int available() override;
    int read() override;

    // Write is not supported for this receive-only implementation
    void write(const uint8_t* data, size_t len) override;

private:
    PIO _pio;
    uint _sm;
    uint _rxPin;
    uint _offset;
};

#endif // PIO_SOFT_SERIAL_H
