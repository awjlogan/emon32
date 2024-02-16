# emonPi3

_emonPi3_ is a drop in replacement for the [emonPi2](https://docs.openenergymonitor.org/emonpi2/index.html). The main change is the use of a modern high performance microcontroller in place of the aging and expensive AVR parts. Additionally, the firmware moves away from using Arduino as the base environment.

## Configuration



## Firmware

Firmware for the emonPi3 is available [here](https://github.com/awjlogan/emon32-fw). The emonPi3 comes preloaded with a USB bootloader, allowing the firmware to be updated without any specialised hardware.

## Changelog

### 0.1

  - Add ADC calibration hardware
  - Add testpoints
  - Move EEPROM and reassign serial modules
  - Add user button, LEDs, external I2C header

### 0.1-dev

  - First bring up of the emonPi3. Directly physically compatible with the existing emonPi2. 
  - Change to 4-layer board
  - Add extra filters and power protection

