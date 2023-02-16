#include "emon32_samd.h"

void
sercomSetup()
{
    /* Debug UART setup */
    portPinMux(PIN_UART_DBG_TX, PORT_PMUX_PMUXE_D);
    portPinMux(PIN_UART_DBG_RX, PORT_PMUX_PMUXE_D);

    const uint32_t baud_dbg = UART_DBG_BAUD;
    const uint64_t br_dbg = (uint64_t)65536 * (F_PERIPH - 16 * baud_dbg) / F_PERIPH;

    /* Configure clocks - runs from the OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= SERCOM_UART_DBG_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_UART_DBG_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure the USART */
    SERCOM_UART_DBG->USART.CTRLA.reg =   SERCOM_USART_CTRLA_DORD
                                       | SERCOM_USART_CTRLA_MODE_USART_INT_CLK
                                       | SERCOM_USART_CTRLA_RXPO(UART_DBG_PAD_RX)
                                       | SERCOM_USART_CTRLA_TXPO(UART_DBG_PAD_TX);

    /* TX/RX enable requires synchronisation */
    SERCOM_UART_DBG->USART.CTRLB.reg =   SERCOM_USART_CTRLB_RXEN
                                       | SERCOM_USART_CTRLB_TXEN
                                       | SERCOM_USART_CTRLB_CHSIZE(0);
    while (SERCOM_UART_DBG->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_CTRLB);

    SERCOM_UART_DBG->USART.BAUD.reg = (uint16_t)br_dbg + 1u;

    /* Enable requires synchronisation (25.6.6) */
    SERCOM_UART_DBG->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
    while (SERCOM_UART_DBG->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_ENABLE);

#ifndef EEPROM_EMULATED

    /* I2C Setup */
    portPinMux(PIN_I2C_SDA, PORT_PMUX_PMUXE_C);
    portPinMux(PIN_I2C_SCL, PORT_PMUX_PMUXE_C);

    PM->APBCMASK.reg |= SERCOM_I2CM_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_I2CM_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* For 400 kHz I2C, SCL T_high >= 0.6 us, T_low >= 1.3 us, with
     * (T_high + T_low) <= 2.5 us, and T_low / T_high ~ 1.8.
     * From I2C->Clock generation (27.6.2.4):
     * BAUD.BAUDLOW = (T_low * f_clk) - 5 (1.625 us -> 8 @ 8 MHz)
     * BAUD.BAUD = (T_high * f_clk) - 5 (0.875 us -> 2 @ 8 MHz)
     */
    SERCOM_I2CM->I2CM.BAUD.reg =   SERCOM_I2CM_BAUD_BAUDLOW(8u)
                                | SERCOM_I2CM_BAUD_BAUD(2u);

    /* Configure the master I2C SERCOM */
    SERCOM_I2CM->I2CM.CTRLA.reg =   SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;

    /* Enable SERCOM, with sync */
    SERCOM_I2CM->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
    while (SERCOM_I2CM->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);

    /* After enabling the I2C SERCOM, the bus state is UNKNOWN (Table 27.13)
     * Force into IDLE state, with sync
     */
    SERCOM_I2CM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(0x1u);
    while (SERCOM_I2CM->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);

    SERCOM_I2CM->I2CM.INTENSET.reg =   SERCOM_I2CM_INTENSET_MB
                                     | SERCOM_I2CM_INTENSET_SB
                                     | SERCOM_I2CM_INTENSET_ERROR;

#endif /* EEPROM_EMULATED */

    /* Data transmitter setup. When using RFM69 modules, this should be
     * configured as SPI. If using ESP8266, then this should be a UART
     */

#ifdef TRANSMIT_ESP8266

    portPinMux(PIN_UART_DATA_TX, PORT_PMUX_PMUXE_D);

    const uint32_t baud_data = UART_DATA_BAUD;
    const uint64_t br_data = (uint64_t)65536 * (F_PERIPH - 16 * baud_data) / F_PERIPH;

    /* Configure clocks - runs from the OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= SERCOM_UART_DATA_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_UART_DATA_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure the USART */
    SERCOM_UART_DATA->USART.CTRLA.reg =   SERCOM_USART_CTRLA_DORD
                                        | SERCOM_USART_CTRLA_MODE_USART_INT_CLK
                                        | SERCOM_USART_CTRLA_TXPO(UART_DBG_PAD_TX);

    /* TX/RX enable requires synchronisation */
    SERCOM_UART_DATA->USART.CTRLB.reg =   SERCOM_USART_CTRLB_TXEN
                                        | SERCOM_USART_CTRLB_CHSIZE(0);
    while (SERCOM_UART_DATA->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_CTRLB);

    SERCOM_UART_DATA->USART.BAUD.reg = (uint16_t)br_data + 1u;

    /* Enable requires synchronisation (25.6.6) */
    SERCOM_UART_DATA->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
    while (SERCOM_UART_DATA->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_ENABLE);

#endif /* TRANSMIT_ESP8266 */

}

/*
 * =====================================
 * UART Functions
 * =====================================
 */

void
uartPutcBlocking(Sercom *sercom, char c)
{
    while (!(sercom->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
    sercom->USART.DATA.reg = c;
    sercom->USART.INTFLAG.reg = 0;
}

void
uartPutsBlocking(Sercom *sercom, const char *s)
{
    while (*s) uartPutcBlocking(sercom, *s++);
}

void
uartConfigureDMA()
{
    volatile DmacDescriptor * dmacDesc = dmacGetDescriptor(DMA_CHAN_UART_DBG);
    dmacDesc->BTCTRL.reg =   DMAC_BTCTRL_VALID
                           | DMAC_BTCTRL_BLOCKACT_NOACT
                           | DMAC_BTCTRL_STEPSIZE_X1
                           | DMAC_BTCTRL_STEPSEL_SRC
                           | DMAC_BTCTRL_SRCINC
                           | DMAC_BTCTRL_BEATSIZE_BYTE;

    dmacDesc->DSTADDR.reg = (uint32_t)&SERCOM_UART_DBG->USART.DATA;
    dmacDesc->DESCADDR.reg = 0u;
}

void
uartPutsNonBlocking(unsigned int dma_chan, const char * const s, uint16_t len)
{
    volatile DmacDescriptor * dmacDesc = dmacGetDescriptor(dma_chan);
    /* Valid bit is cleared when a channel is complete */
    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = len;
    dmacDesc->SRCADDR.reg = (uint32_t)s + len;
    dmacStartTransfer(dma_chan);
}

char
uartGetc(const Sercom *sercom)
{
    return sercom->USART.DATA.reg;
}

void
uartInterruptEnable(Sercom *sercom, uint32_t interrupt)
{
    sercom->USART.INTENSET.reg |= interrupt;
}

void
uartInterruptDisable(Sercom *sercom, uint32_t interrupt)
{
    sercom->USART.INTENCLR.reg |= interrupt;
}

uint32_t
uartInterruptStatus(const Sercom *sercom)
{
    return sercom->USART.INTFLAG.reg;
}

void
uartInterruptClear(Sercom *sercom, uint32_t interrupt)
{
    sercom->USART.INTFLAG.reg |= interrupt;
}

/*
 * =====================================
 * I2C Functions
 * =====================================
 */

void
i2cActivate(Sercom *sercom, uint8_t addr)
{
    sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
    while (!(sercom->I2CM.INTFLAG.reg & (SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB)));
}

void
i2cAck(Sercom *sercom, I2CM_Ack_t ack, I2CM_AckCmd_t cmd)
{
    sercom->I2CM.CTRLB.reg =   (ack << SERCOM_I2CM_CTRLB_ACKACT_Pos)
                             | SERCOM_I2CM_CTRLB_CMD(cmd);
    while(sercom->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);
}

void
i2cDataWrite(Sercom *sercom, uint8_t data)
{
    sercom->I2CM.DATA.reg = data;
    while (!(sercom->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));
}

uint8_t
i2cDataRead(Sercom *sercom)
{
    while (!(sercom->I2CM.INTFLAG.reg & (SERCOM_I2CM_INTFLAG_MB | SERCOM_I2CM_INTFLAG_SB)));
    return sercom->I2CM.DATA.reg;
}

/*
 * =====================================
 * SPI Functions
 * =====================================
 */

void
spiWriteByte(Sercom *sercom, const spiPkt_t *pPkt)
{

}

void
spiReadByte(Sercom *sercom, spiPkt_t *pPkt)
{

}
