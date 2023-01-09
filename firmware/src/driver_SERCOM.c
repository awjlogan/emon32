#include "emon32_samd.h"

void
sercomSetup()
{
    portPinMux(PIN_UART_TX, PORT_PMUX_PMUXE_D);
    portPinMux(PIN_UART_RX, PORT_PMUX_PMUXE_D);

    //const uint32_t baud = UART_DBG_BAUD;
    //const uint64_t br = (uint64_t)65536 * (F_PERIPH - 16 * baud) / F_PERIPH;
    const uint32_t baud = (F_PERIPH * 8) / (16 * 38400);

    /* Configure clocks - runs from the OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= SERCOM_UART_DBG_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_UART_DBG_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure the USART */
    SERCOM_UART_DBG->USART.CTRLA.reg =   SERCOM_USART_CTRLA_DORD
                                       | SERCOM_USART_CTRLA_SAMPR(1u)
                                       | SERCOM_USART_CTRLA_MODE_USART_INT_CLK
                                       | SERCOM_USART_CTRLA_RXPO(UART_DBG_PAD_RX)
                                       | SERCOM_USART_CTRLA_TXPO(UART_DBG_PAD_TX);

    /* TX/RX enable requires synchronisation */
    SERCOM_UART_DBG->USART.CTRLB.reg =   SERCOM_USART_CTRLB_RXEN
                                       | SERCOM_USART_CTRLB_TXEN
                                       | SERCOM_USART_CTRLB_CHSIZE(0);
    while (SERCOM_UART_DBG->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_CTRLB);

    SERCOM_UART_DBG->USART.BAUD.FRAC.FP = baud % 8;
    SERCOM_UART_DBG->USART.BAUD.FRAC.BAUD = baud / 8;
    //SERCOM_UART_DBG->USART.BAUD.reg = (uint16_t)br + 1u;

    /* Enable requires synchronisation (25.6.6) */
    SERCOM_UART_DBG->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
    while (SERCOM_UART_DBG->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_ENABLE);

#ifndef EEPROM_EMULATED

    portPinMux(PIN_I2C_SDA, PORT_PMUX_PMUXE_C);
    portPinMux(PIN_I2C_SCL, PORT_PMUX_PMUXO_C);

    PM->APBCMASK.reg |= SERCOM_I2CM_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_I2CM_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* TODO configure baud rate */
    /* Configure the master I2C SERCOM for external EEPROM */
    SERCOM_I2CM->I2CM.CTRLA.reg =   SERCOM_I2CM_CTRLA_MODE_I2C_MASTER
                                  /* Table 27.11 : 300-600 ns SDA hold */
                                  | SERCOM_I2CM_CTRLA_SDAHOLD(0x2u);

    /* Enable Smart Mode to issue (N)ACK automatically (27.6.3.2)
     * Wait for synchronisation on write to CTRLB (27.8.2.8)
     */
    SERCOM_I2CM->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_SMEN;
    while (SERCOM_I2CM->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);

    /* Enable SERCOM, with sync */
    SERCOM_I2CM->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
    while (SERCOM_I2CM->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);

    /* After enabling the I2C SERCOM, the bus state is UNKNOWN (Table 27.13)
     * Force into IDLE state, with sync
     */
    SERCOM_I2CM->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(0x1u);
    while (SERCOM_I2CM->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_SYSOP);

#endif

}

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

void
i2cActivate(Sercom *sercom, unsigned int addr, unsigned int dma, unsigned int len)
{
    if (0 == dma)
    {
        sercom->I2CM.ADDR.reg = SERCOM_I2CM_ADDR_ADDR(addr);
    }
    else
    {
        sercom->I2CM.ADDR.reg =   SERCOM_I2CM_ADDR_ADDR(addr)
                                | SERCOM_I2CM_ADDR_LEN(len)
                                | SERCOM_I2CM_ADDR_LENEN;
    }
}
