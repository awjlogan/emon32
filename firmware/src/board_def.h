#ifndef BOARD_DEF_H
#define BOARD_DEF_H

/* This file contains defines specific to the board that is being used. To
 * extend this, the base SAMD10 configuration can be bracketed in IFDEF
 */

/* Clock frequencies
 *  - Core is on the 48 MHz DFLL
 *  - Peripherals are on the OSC8M / 8 -> MHz
 */
#define     F_CORE      48000000ul
#define     F_PERIPH    8000000ul

/* Pin assignments (nb. logical, not physical) */
#define PIN_EXTINT          24u
#define PIN_GEN_STATUS      25u
#define PIN_LED             16u
#define PIN_SW              15u
#define PIN_UART_RX         9u
#define PIN_UART_TX         8u

/* UART related defines */
#define SERCOM_UART         SERCOM0
#define UART_PAD_RX         3u
#define UART_PAD_TX         1u
#define UART_BAUD           38400u

/* SAMD10 calibration values */
#define CAL_REG             *(const volatile uint32_t *)(0x00806024)
#define CAL_OSC32_Msk       0x1FC0u
#define CAL_OSC32_Pos       6u

#endif
