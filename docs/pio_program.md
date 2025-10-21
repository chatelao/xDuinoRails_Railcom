# PIO Program for RailCom Cutout

The `railcom.pio` program is responsible for generating the precise 454µs cutout in the DCC signal that is required for RailCom communication. This is achieved by using the RP2040's Programmable I/O (PIO) peripheral.

## How it Works

The program uses a series of `nop` instructions with carefully calculated delays to create the cutout. The `set pins` instruction is used to drive the DCC signal high and low. An IRQ is used to signal the CPU when the cutout is complete.

## Timing

The system clock is assumed to be 125MHz. The target duration is 454µs, which is 56750 cycles. The achieved duration is 56737 cycles, which is 453.896µs. This is well within the tolerance of the RailCom specification.
