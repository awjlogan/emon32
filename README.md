# Energy Monitor 32

## Introduction

_Energy Monitor 32_ (emon32) is a system for measuring power consumption using _current transfomers_ (CTs). It is based heavily on the work done by [OpenEnergyMonitor](https://openenergymonitor.org), particularly the [emonTx](https://github.com/openenergymonitor/emontx4). It is designed to be compatible with the downstream data requirements of that system.

The emonTx energy monitors are currently based around the ATMega328, as made popular by Arduino, and an extended version. These chips are expensive, have few peripherals, and are being pushed beyond the limit of their data processing capability. The aims of this project are:

  - Provide a low cost, high performance energy monitoring system.
  - Provide a scalable and modular system.
  - Provide an architecture agnostic library to handle energy monitoring functions.

## Getting started

### Hardware design (Low Cost)

A _Low Cost_ reference design is included. This is a simple 3 CT channel system. It includes the following improvements over the original emonTx implementation:

  - Virtual ground is buffered, and shared across all V/CT channels
  - Differential ADC with precision reference
  - Oversampling with anti aliasing filter

### Compiling and uploading the firmware

The firmware can be compiled from the `firmware` directory by running `make`. You will need the toolchain corresponding to your target microcontroller. Further details are given in the [firmware readme](firmware/README.md).

The firmware is then flashed to the microcontoller using, for example, openOCD. This step is specific to the microcontroller and board used.

### Configuring the system

The default settings provide a viable system compatible with the emonTx system. Details regarding _compile time_ configurable options is given in the [firmware readme](firmware/README.md). To configure _run time_ options:

  1. Connect the debug UART to a host
  2. Reset or power on the emon32
  3. When prompted, within 3 s, enter any key to access the configuration menu
  4. When complete, the configuration is saved to non-volatile memory, or discarded.

The following settings are configurable at run time:

  - Mains frequency
  - Equilibration time (discard a number of cycles)
  - Report time
  - Per channel calibration values
    - (V/CT) Scaling factor
    - (CT only) Phase correction

The following information is also provided:

  - Firmware version
  - Serial number
  - Number of V/CT channels

Note that all run time configurable options can be preset before compiling. The values provided are sensible defaults, and these values remain configurable at all times.

## Monitoring Architecture

The firmware uses an event driven system to gather voltage and current data in real time, and handle processing and transmission asynchronously. This means:

  - (RT) Raw data are accummulated, with preliminary processing
  - (Async) When a mains cycle is complete (zero crossing), the accumulated data are processed into RMS and real power
  - (Async) When a configurable number of cycles have been processed, the aggregate data is then transmitted

### Microcontroller Selection

The firmware is agnostic to the microcontoller used, although a modern 32 bit core is preferred. This implementation targets the [Microchip SAMD series](https://www.microchip.com/en-us/products/microcontrollers-and-microprocessors/32-bit-mcus/sam-32-bit-mcus/sam-d) - this allows easy porting from small and cheap (SAMD1x) to higher performance and capability microcontrollers, such as the SAMD2x and SAMD5x. The SAMD family has the following useful functions

  - Differential ADC (dual channel for SAMD5x)
  - Asynchronous event system
    - This allows precise sample timing without any interrupt overhead
  - Generic serial communication modules
  - Flexible clocking system
    - A 48 MHz PLL is used for the core, and peripherals are clocked from an RC oscillator
  - USB virtual serial port (SAMD21 and SAMD51 only)

If you would like to use a different microcontroller, the following features are recommended:

  - Minimum 24 MHz clock
  - Timer with 1 us resolution
  - DMA (minimum 2 channel)
  - 2x UART module
  - SPI module
  - I2C module

Functions in the main loop are abstacted from the underlying hardware. Implementation specific configuration (for example, configuring the ADC) must be provided.

## License

This project is licensed under [GPL v3.0](https://www.gnu.org/licenses/gpl-3.0-standalone.html) or later. Any alterations must be made available immediately, or upon request.

## Acknowledgements

  - The SAMD10 bare metal environment was taken from the excellent collection of [microcontroller starter projects](https://github.com/ataradov/mcu-starter-projects) by Alex Taradov
