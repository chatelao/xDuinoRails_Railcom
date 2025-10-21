# Implementation Details

## PIO-based Cutout

The RailCom cutout is generated using the RP2040's PIO. The PIO is a powerful feature that allows you to create custom peripherals. In this library, we use the PIO to create a precise 454us cutout on the TX pin.

The PIO program is written in assembly and is located in the `src/railcom.pio` file. The program is very simple. It waits for a value to be pushed to the FIFO, and then it sets the TX pin to high for 454us. After the cutout, it sets the TX pin to low and sends an interrupt to the CPU.

The interrupt handler is located in the `RailcomSender` class. When the interrupt is received, the handler re-enables the UART and sends the DCC message.

## Automatic Locomotive Detection

The automatic locomotive detection protocol is implemented in the `RailcomReceiver` class. The protocol is very simple. The receiver listens for a discovery request from the central station. The discovery request is a broadcast message with the address 0xFFFF and the command 0x01.

When the receiver receives a discovery request, it responds with its address. The address is sent as a 2-byte message. The first byte is the high byte of the address, and the second byte is the low byte of the address.

The central station can then use the address to communicate with the locomotive.
