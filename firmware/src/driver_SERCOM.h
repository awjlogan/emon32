#ifndef DRIVER_SERCOM_H
#define DRIVER_SERCOM_H

#include "samd10.h"
#include <stdint.h>

typedef enum {
    I2CM_ACK    = 0u,
    I2CM_NACK   = 1u
} I2CM_Ack_t;

typedef enum {
    I2CM_ACK_CMD_NONE       = 0u,
    I2CM_ACK_CMD_START      = 1u,
    I2CM_ACK_CMD_CONTINUE   = 2u,
    I2CM_ACK_CMD_STOP       = 3u
} I2CM_AckCmd_t;

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
 */
void i2cActivate(Sercom *sercom, uint8_t addr);

/*! @brief Requester acknowledge command
 *  @param [in] sercom : SERCOM instance
 *  @param [in] ack : 0: ACK, 1: NACK
 *  @param [in] cmd : command
 */
void i2cAck(Sercom *sercom, I2CM_Ack_t ack, I2CM_AckCmd_t cmd);

/*! @brief Write to completer
 *  @param [in] sercom : SERCOM instance
 *  @param [in] data : data byte
 */
void i2cDataWrite(Sercom *sercom, uint8_t data);

/*! @brief Read byte from I2C completer
 *  @param [in] : sercom : SERCOM instance
 *  @return : read data
 */
uint8_t i2cDataRead(Sercom *sercom);

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
