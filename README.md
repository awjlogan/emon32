# Energy Monitor 32

## Introduction

_Energy Monitor 32_ (emon32) is a system for measuring power consumption using _current transfomers_ (CTs). It is based heavily on the work done by [OpenEnergyMonitor](https://openenergymonitor.org), particularly the [emonTx](https://github.com/openenergymonitor/emontx4). It is designed to be compatible with the downstream data requirements of that system.

The emonTx energy monitors are currently based around the ATMega328, as made popular by Arduino, and an extended version, the AVR128DB. These chips are expensive, have few peripherals, and are being pushed to the limit of their data processing capability. The aims of this project are:

  - Provide a low cost, high performance energy monitoring system.
  - Provide a scalable and modular system.
  - Provide an architecture agnostic library to handle energy monitoring functions.

## Getting started

There are currently two implementations:

  - [emonPi3](pcb/emonpi3/README.md)
  - [Low Cost](pcb/lc/README.md)
    - Not actively maintained as of Feb 2024.

## Microcontroller Selection

The firmware is agnostic to the microcontoller used, although a modern 32 bit core is preferred. This implementation targets the [Microchip SAMD series](https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/32-bit-mcus/sam-32-bit-mcus/sam-d) - this allows easy porting from small and cheap (SAMD1x) to higher performance and capability microcontrollers, such as the SAMD2x and SAMD5x. The SAMD family has the following useful functions

  - Differential ADC (dual channel for SAMD5x)
  - Asynchronous event system
    - This allows precise sample timing without any interrupt overhead
  - Generic serial communication modules
  - Flexible clocking system
    - High speed clocks are available for the core, and separate clocks for each peripheral
    - Clocks and peripherals can be gated when not in use
  - USB virtual serial port (SAMD21 and SAMD51 only)
  - USB bootloader (SAMD11, SAMD21, SAMD51 only)

If you would like to use a different microcontroller, the following features are recommended:

  - Minimum 24 MHz clock
  - Timer with 1 us resolution
  - DMA (minimum 2 channels)
  - 2x UART module
  - SPI module
  - I2C module

Functions in the main loop are abstacted from the underlying hardware. Implementation specific configuration (for example, configuring the ADC) must be provided.

## License

This project is licensed under the [Creative Commons Attribution Sharealike 4.0](https://creativecommons.org/licenses/by-sa/4.0/deed.en). Any alterations must be made available immediately, or upon request.

## Acknowledgements

  - OpenEnergyMonitor
  - Rob Wall of Open Energy Monitor forums for lots of in depth discussion around sampling strategies.

