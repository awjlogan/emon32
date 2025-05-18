# emonPi3 Changelog

## v0.2 -> v1.0

First production run.

- [x] Fix footprint for LED
- [x] Update design rules for Aisler 4-layer
- [x] Minor revisions to footprints
- [x] Tidy silkscreen

## v0.1 -> v0.2

Production candidate

- [x] Change to 40 pin RasPi header
- [x] Add ESD protection for buttons and pulse/analog inputs
- [x] Split ANALOG_INPUT and CT12 inputs
- [x] Move STATUS and PROG LEDs to combined front panel
- [x] Remove USER LEDs and switch
- [x] Swap SPI MOSI and !SS, incorrect in v0.1
- [x] Retouch silkscreen boxes to avoid overlap
- [x] Make OneWire pull up selectable in software
- [x] Mounting support for OLED
- [x] Disable external interfaces from Pi
- [x] Add support to reset from Pi
- [x] Remove hardware zero crossing detection
- [x] Change expander board header for 2x5
- [x] Teardrops on all traces
- [x] Revise routing, particularly on inner 1

## v0.1-dev -> emonPi3 v0.1

- [x] Change to SMD mount for RPi
- [x] Change Cortex-M header to SMD
- [x] Move protection diodes closer to jacks
- [x] Add V-USB detection
- [x] Add hardware zero crossing detection
  - Added comparator and moved STATUS and PROG LED assignments
- [x] Mechanical support for expander board
- [x] Add board version on [38, 41, 42]
  - Can't do any more routing with these pins, gives 8 revisions
  - No need to change for non-s/w visible changes
- [x] Change SPI traces to 50R impedance (245 um)
- [x] Add AA filter on ANALOG_INPUT
- [x] Fix spacing for pulse/temperature header
- [x] Tidy schematic
- [x] Move jumpers to top side for wave soldering
- [x] EEPROM WP should connect to ground
- [x] Move jumpers to top layer (allows wave soldering)
- [x] Add 10K pullup for !RESET
- [x] ADC calibration requires 1:2:1 ratio for divider
  - Extra 5K1 0.1% resistor in the middle
- [x] Better labelling for test points
- [x] Add SWD testpads
- [x] Adjust information text
  - [x] Repository information
- [x] Change programming switch to standard 6x6 mm
- [x] Reposition EEPROM and bring out I2C
  - [x] EXT I2C now position of INT I2C
  - [x] UART -> EXT I2C
  - [x] SPI -> UART
  - [x] INT I2C -> SPI
- [x] Change ANALOG_EXPANDER and J7 to sockets
- [x] LEDs -> active LOW
- [x] Extra indication LEDs
  - [x] 2X General purpose from SAMD
- [x] Change OneWire pull up to 3K3 (reduce BoM count)
- [x] Check spacing of SWD header and RST SW
- [x] Add I2C header for INT
- [x] RasPi present signal
- [x] Swap labels for UART  Rx/Tx header
- [x] Rename to emonPi3

## emonPi2 v2.0 -> emon32-Pi2 v0.1-dev

- Convert to KiCad format
- 4 layers
- Keepouts on slide rail sides
- Change bias from 3V3 to V_REF
- Added
  - ATSAMD21
  - Cortex-M debug header
  - switch protection
  - anti-alias filter
  - USB PD negotiation resistors
  - USB ESD protection
  - Testpoints
    - TP1: V_REF
    - TP2: V_MID
    - TP3: 5V0
    - TP4: 3V3
- Removed
  - CP2102 (SAMD has onboard USB)
  - AVR & programming header
