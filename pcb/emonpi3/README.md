# emonPi3

_emonPi3_ is a drop in replacement for the [emonPi2](https://docs.openenergymonitor.org/emonpi2/index.html). The main change is the use of a modern high performance microcontroller in place of the aging and expensive AVR parts. Additionally, the firmware moves away from using Arduino as the base environment.

## Firmware

Firmware for the emonPi3 is available [here](https://github.com/awjlogan/emon32-fw). The emonPi3 comes preloaded with a USB bootloader, allowing the firmware to be updated without any specialised hardware.

## Generating manufacturing outputs

You will need to have [KiCad 7](https://www.kicad.org/) installed. In the `emonpi3` folder, run `python3 generate.py`. This will make a folder called `output-<gitrev>` with the following contents:

  - KiCad schematic annotated with git revision
  - KiCad PCB annotated with git revision
  - Schematic PDF
  - Floorplan PDF
  - Bill of materials CSV
  - Gerbers output
  - 3D render (STEP format)

This has been tested on Linux and macOS.

## Changelog

### 0.1

First public limited manufacturing run.

  - Add hardware zero crossing detector.
  - Add ADC calibration hardware.
  - Add more testpoints.
  - Move EEPROM and reassign serial modules.
  - Add user button, LEDs, and external I2C header.

### 0.1-dev

  - First bring up of the emonPi3. 
    - Directly physically compatible with the existing emonPi2. 
  - Change to 4-layer board
  - Add extra filters and power protection

