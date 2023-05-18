#ifndef BOARD_DEF_H
#define BOARD_DEF_H

/* This file contains defines specific to the board that is being used. To
 * extend this, the base SAMD10 configuration can be bracketed in IFDEF
 */

/* Online configuration takes around 2.5 KB of flash. Comment out this define to
 * save space, with only RO access to configuration values.
 */
#define CONFIGURATION_RW

/* Clock frequencies
 *  - Core is on the 48 MHz DFLL
 *  - Peripherals are on the OSC8M / 8 -> MHz
 */
#define F_CORE              48000000ul
#define F_PERIPH            8000000ul
#define F_TC1               F_PERIPH / 8
#define F_TC2               F_PERIPH / 8

/* Analog comparator (AC) for zero crossing detection. Uncomment if the AC is
 * being used for zero crossing. If not present, a software routine is used
 */
/* #define ZERO_CROSSING_HW */

/* EEPROM */
/* Uncomment to use the software EEPROM mode (not implemented yet)
 * #define EEPROM_EMULATED
 */
/* Top of EEPROM address, not including R/W bit */
#define EEPROM_BASE_ADDR    0x50
/* Maximum number of bytes in a single page */
#define EEPROM_PAGE_SIZE    16u
/* Worst case EEPROM write time (microseconds) */
#define EEPROM_WR_TIME      5000ul
/* Size (bytes) of EEPROM */
#define EEPROM_SIZE_BYTES   512u
/* Size (bytes) of wear levelled portion */
#define EEPROM_WL_SIZE      384u
/* Offset of wear levelled area */
#define EEPROM_WL_OFFSET    (EEPROM_SIZE_BYTES - EEPROM_WL_SIZE)

/* SERCOM peripheral defines */
#define SERCOM_UART_DBG             SERCOM0
#define SERCOM_I2CM                 SERCOM1
#define SERCOM_UART_DATA            SERCOM2
#define SERCOM_SPI_DATA             SERCOM2

#define SERCOM_UART_DBG_APBCMASK    PM_APBCMASK_SERCOM0
#define SERCOM_I2CM_APBCMASK        PM_APBCMASK_SERCOM1
#define SERCOM_UART_DATA_APBCMASK   PM_APBCMASK_SERCOM2

#define SERCOM_UART_DBG_GCLK_ID     SERCOM0_GCLK_ID_CORE
#define SERCOM_I2CM_GCLK_ID         SERCOM1_GCLK_ID_CORE
#define SERCOM_UART_DATA_GCLK_ID    SERCOM2_GCLK_ID_CORE

#define SERCOM_UART_DBG_DMAC_ID_TX  SERCOM0_DMAC_ID_TX
#define SERCOM_I2CM_DMAC_ID_TX      SERCOM2_DMAC_ID_TX
#define SERCOM_I2CM_DMAC_ID_RX      SERCOM2_DMAC_ID_RX
#define SERCOM_UART_DATA_DMAC_ID_TX SERCOM2_DMAC_ID_TX

#define SERCOM_UART_DBG_NVIC_IRQn   SERCOM0_IRQn

/* Pulse counting */
#define PULSE_MIN_PERIOD_MS         100u    /* Minimum period between pulses */
#define PULSE_EIC_MAP               3u
#define PULSE_EIC_FILTER            EIC_CONFIG_FILTEN3
#define PULSE_EIC_RISING            EIC_CONFIG_SENSE3_RISE
#define PULSE_EIC_INTFLAG           EIC_INTFLAG_EXTINT3
#define PULSE_EIC_INTENSET          EIC_INTENSET_EXTINT3
#define PULSE_EIC_INTENCLR          EIC_INTENCLR_EXTINT3

/* Pin assignments (nb. logical, not physical) */
#define PIN_EXTINT          24u
#define PIN_GEN_STATUS      25u
#define PIN_LED             16u
#define PIN_SW              15u
#define PIN_UART_DBG_RX     9u
#define PIN_UART_DBG_TX     8u
#define PIN_I2C_SDA         22u
#define PIN_I2C_SCL         23u

/* Debug UART related defines */
#define UART_DBG_PAD_RX     3u
#define UART_DBG_PAD_TX     1u
#define UART_DBG_BAUD       38400u

/* SPI related defines */
#define PIN_SPI_MISO        14u
#define PIN_SPI_MOSI        15u
#define PIN_SPI_SCK         16u
#define PIN_SPI_RFM_SS      17u
#define SPI_DATA_BAUD       4000000ul

/* DMA defines */
#define NUM_CHAN_DMA        4u
#define DMA_CHAN_UART_DATA  3u
#define DMA_CHAN_I2CM       2u
#define DMA_CHAN_UART_DBG   1u
#define DMA_CHAN_ADC        0u

/* SAMD10 calibration values (NVM User Row Mapping Table 9-3) */
#define CAL_REG_LOW         *(const volatile uint32_t *)(0x00806020)
#define CAL_REG_HIGH        *(const volatile uint32_t *)(0x00806024)
#define CAL_OSC32_Msk       0x1FC0u
#define CAL_OSC32_Pos       6u
#define CAL_ADC_BIAS_Msk    0x38u
#define CAL_ADC_BIAS_Pos    3u
#define CAL_ADC_LIN_L_Msk   0xF800u
#define CAL_ADC_LIN_L_Pos   27u
#define CAL_ADC_LIN_H_Msk   0x7u

#endif
