# 🔌 emon _32_ firmware 🔌

## Compiling

Compiling the firmware requires the correct toolchain. The example below is for a Cortex-M system, specifically the Atmel ATSAMD10D14. You will need:

TODO List of requirements

  - [ ]

To build the firmware:

  `firmware > make`

This will generate `firmware/build/emon32.elf` which can then be flashed to microcontroller.

## Structure and modifications

 - All peripheral drivers are in header/source pairs named **driver_<PERIPHERAL>**. For example, the ADC driver is in **driver_ADC.\***.
