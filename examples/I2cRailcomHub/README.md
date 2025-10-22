# I2C Railcom Hub Example

This example demonstrates how to use the RP2040 or RP2350 to create an I2C Railcom hub. The hub monitors multiple Railcom channels and makes the latest Railcom data, along with the associated DCC message, available to an I2C master.

## Channel Capacity

*   **RP2040:** 10 channels (2 hardware UARTs + 8 PIO UARTs)
*   **RP2350:** 14 channels (2 hardware UARTs + 12 PIO UARTs)

## Hardware Setup

*   Connect the DCC signal to GPIO pin 6.
*   Connect the Railcom signals for the hardware UARTs to the following pins:
    *   Channel 1 (uart0): GPIO 2
    *   Channel 2 (uart1): GPIO 4
*   Connect the Railcom signals for the PIO UARTs to the following pins:
    *   Channel 3: GPIO 9
    *   Channel 4: GPIO 10
    *   ...and so on, up to the maximum number of channels for your board.
*   Connect the I2C bus (SDA and SCL) to the I2C master. The I2C address of the hub is 0x08.

## How it Works

The example uses the `NmraDcc` library to parse the DCC signal. When a DCC packet is received, the hub reads the Railcom messages from all channels and stores them in a buffer, along with the DCC message.

When an I2C master requests data from the hub, the hub sends the contents of the buffers for all channels. The data is sent as a raw byte array, with the first byte indicating the size of the message.

## Customization

You can customize the pins used for the Railcom channels by modifying the `setup()` function in the `I2cRailcomHub.ino` file.
