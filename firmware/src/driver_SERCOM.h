#ifndef DRIVER_SERCOM_H
#define DRIVER_SERCOM_H

#include "samd10.h"
#include <stdint.h>

typedef struct {
    uint8_t addr;
    uint8_t data;
} spiPkt_t;

/* @brief configure the serial communication modules */
void sercomSetup();

/*! @brief Send a single character (blocking) on UART
 *  @param [in] sercom : pointer to the SERCOM instance
 *  @param [in] c : Single character
 */
void uartPutcBlocking(Sercom *sercom, char c);

/*! @brief Send a string (blocking) on UART
 *  @param [in] sercom : pointer to the SERCOM instance
 *  @param [in] s : Pointer to null terminated string
 */
void uartPutsBlocking(Sercom *sercom, const char *s);

/*! @brief Configure the DMA for non-blocking transactions
 */
void uartConfigureDMA();

/*! @brief Send a string (non-blocking) on UART by DMA
 *  @param [in] dma_chan : DMA channel to send on
 *  @param [in] s : Pointer to the string
 *  @param [in] len : Length of the string (not including NULL)
 */
void uartPutsNonBlocking(unsigned int dma_chan, const char * const s, uint16_t len);

/*! @brief Get a character from the USART data buffer. Only valid when the
 *         INTFLAG.RXC bit it set.
 *  @param [in] sercom : SERCOM instance
 */
char uartGetc(const Sercom *sercom);

/*! @brief Enable the an interrupt for the UART instance
 *  @param [in] sercom : SERCOM instance
 *  @param [in] interrupt : interrupt to enable
 */
void uartInterruptEnable(Sercom *sercom, uint32_t interrupt);

/*! @brief Disable the an interrupt for the UART instance
 *  @param [in] sercom : SERCOM instance
 *  @param [in] interrupt : interrupt to disable
 */
void uartInterruptDisable(Sercom *sercom, uint32_t interrupt);

/*! @brief Return the interrupt status for the UART instance
 *  @param [in] sercom : SERCOM instance
 */
uint32_t uartInterruptStatus(const Sercom *sercom);

/*! @brief Clear the interrupt status for the UART instance
 *  @param [in] sercom : SERCOM instance
 *  @param [in] interrupt : interrupt to clear
 */
void uartInterruptClear(Sercom *sercom, uint32_t interrupt);

/*! @brief Set I2C address. If dma is 1, then a packet of len bytes is sent
 *         or received.
 *  @param [in] sercom : SERCOM instance
 *  @param [in] addr : address and RW bit
 *  @param [in] dma  : send packet by DMA
 *  @param [in] len  : number of bytes to send
 */
void i2cActivate(Sercom *sercom, unsigned int addr, unsigned int dma, unsigned int len);

/*! @brief Write a byte to a configured SPI channel
 *  @param [in] sercom : SERCOM instance
 *  @param [in] pPkt : pointer to SPI packet
 */
void spiWriteByte(Sercom *sercom, const spiPkt_t *pPkt);

/*! @brief Read a byte from a configured SPI channel
 *  @param [in] sercom : SERCOM instance
 *  @param [in] pPkt : pointer to SPI packet
 */
void spiReadByte(Sercom *sercom, spiPkt_t *pPkt);

#endif
