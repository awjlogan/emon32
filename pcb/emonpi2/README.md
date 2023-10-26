# emon32-Pi2

_emon32-Pi2_ is a drop in replacement for the [emonPi2](URL HERE!). The main change is the use of a modern high performance microcontroller in place of the aging and expensive AVR parts. Additionally, the firmware moves away from using Arduino as the base environment.

## Changelog

### 0.1

  - Add ADC calibration hardware
  - Add testpoints
  - Move EEPROM and reassign serial modules
  - Add user button, LEDs, external I2C header
  - Safe power negotiation with RPi

### 0.1-dev

  - First bring up of the emon32-Pi2. Directly physically compatible with the existing emonPi2. 
  - Change to 4-layer board
  - Add extra filters and power protection
