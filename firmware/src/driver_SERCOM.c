#include "emon32_samd.h"

void
sercomSetup()
{
    /*****************
    * Debug UART setup
    ******************/
    UART_Cfg_t uart_dbg_cfg;
    uart_dbg_cfg.sercom     = SERCOM_UART_DBG;
    uart_dbg_cfg.baud       = UART_DBG_BAUD;
    uart_dbg_cfg.glck_id    = SERCOM_UART_DBG_GCLK_ID;
    uart_dbg_cfg.gclk_gen   = 3u;
    uart_dbg_cfg.pad_tx     = UART_DBG_PAD_TX;
    uart_dbg_cfg.pad_rx     = UART_DBG_PAD_TX;
    uart_dbg_cfg.port_grp   = GRP_SERCOM_UART_DBG;
    uart_dbg_cfg.pin_tx     = PIN_UART_DBG_TX;
    uart_dbg_cfg.pin_rx     = PIN_UART_DBG_RX;
    sercomSetupUART(&uart_dbg_cfg);

    /*****************
    * I2C Setup
    ******************/
    portPinMux(GRP_SERCOM_I2C, PIN_I2C_SDA, PORT_PMUX_PMUXE_C);
    portPinMux(GRP_SERCOM_I2C, PIN_I2C_SCL, PORT_PMUX_PMUXE_C);

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
    SERCOM_I2CM->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;

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
}

void
sercomSetupUART(const UART_Cfg_t *pCfg)
{
    uint16_t baud;
    // const uint64_t br_dbg = (uint64_t)65536 * (F_PERIPH - 16 * pCfg->baud) / F_PERIPH;
    switch (pCfg->baud)
    {
        case (UART_BAUD_9600):
            baud = 64279;
            break;
        case (UART_BAUD_19200):
            baud = 63020;
            break;
        case (UART_BAUD_28800):
            baud = 61762;
            break;
        case (UART_BAUD_38400):
            baud = 60504u;
            break;
        case (UART_BAUD_57600):
            baud = 57987;
            break;
        case (UART_BAUD_76800):
            baud = 55471;
            break;
        case (UART_BAUD_115200):
            baud = 50438u;
            break;
        default:
            /* Default to 9600 if a non-standard baud is entered */
            baud = 64279;
    }

    portPinMux(pCfg->port_grp, pCfg->pin_tx, PORT_PMUX_PMUXE_D);
    portPinMux(pCfg->port_grp, pCfg->pin_rx, PORT_PMUX_PMUXE_D);

    /* Configure clocks - runs from the OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= SERCOM_UART_DBG_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(pCfg->glck_id)
                        | GCLK_CLKCTRL_GEN(pCfg->gclk_gen)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure the USART */
    pCfg->sercom->USART.CTRLA.reg =   SERCOM_USART_CTRLA_DORD
                                    | SERCOM_USART_CTRLA_MODE_USART_INT_CLK
                                    | SERCOM_USART_CTRLA_RXPO(pCfg->pad_rx)
                                    | SERCOM_USART_CTRLA_TXPO(pCfg->pad_tx);

    /* TX/RX enable requires synchronisation */
    pCfg->sercom->USART.CTRLB.reg =   SERCOM_USART_CTRLB_RXEN
                                    | SERCOM_USART_CTRLB_TXEN
                                    | SERCOM_USART_CTRLB_CHSIZE(0);
    while (pCfg->sercom->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_CTRLB);

    pCfg->sercom->USART.BAUD.reg = baud;

    /* Enable requires synchronisation (25.6.6) */
    pCfg->sercom->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
    while (pCfg->sercom->USART.STATUS.reg & SERCOM_USART_SYNCBUSY_ENABLE);
}

void
sercomSetupSPI()
{
    /**********************
    * SPI Setup (for RFM69)
    ***********************/
    portPinMux(GRP_SERCOM_SPI, PIN_SPI_MISO, PORT_PMUX_PMUXE_D);
    portPinMux(GRP_SERCOM_SPI, PIN_SPI_MOSI, PORT_PMUX_PMUXE_D);
    portPinMux(GRP_SERCOM_SPI, PIN_SPI_SCK, PORT_PMUX_PMUXE_D);

    /* Table 24-2 - driven @ F_REF = F_PERIPH */
    const uint32_t baud_data = SPI_DATA_BAUD;
    const uint32_t br_data = ((uint32_t)F_PERIPH / (2 * baud_data) - 1u);

    /* Configure clocks - runs from the OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= SERCOM_UART_DATA_APBCMASK;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM_UART_DATA_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    SERCOM_SPI_DATA->SPI.BAUD.reg = br_data;

    /* SPI mode 0: CPOL == 0, CPHA == 0 */
    SERCOM_SPI_DATA->SPI.CTRLA.reg  = SERCOM_SPI_CTRLA_MODE_SPI_MASTER;

    /* While disabled, RXEN will be set immediately. When the SPI SERCOM is
     * enabled, this requires synchronisation before the SPI is ready. See
     * field description in 26.8.2
     */
    SERCOM_SPI_DATA->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;
    SERCOM_SPI_DATA->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;
    while (0 != SERCOM_SPI_DATA->SPI.SYNCBUSY.reg);
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
spiWriteByte(Sercom *sercom, const uint8_t addr, const uint8_t data)
{
    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_CLR);
    sercom->SPI.DATA.reg = addr;
    while (0 == (sercom->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));
    sercom->SPI.DATA.reg = data;
    while (0 == (sercom->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));
    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_SET);
}

uint8_t
spiReadByte(Sercom *sercom, const uint8_t addr)
{
    /* Set address on first write, then send a dummy byte to provide clock
     * for shifting out the data
     */
    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_CLR);

    sercom->SPI.DATA.reg = addr;
    while (0 == (sercom->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));

    sercom->SPI.DATA.reg = 0;
    while (0 == (sercom->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_RXC));
    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_SET);

    return sercom->SPI.DATA.reg;
}

void
spiWriteBuffer(Sercom *sercom, const void *pBuf, const unsigned int n)
{
    /* Send buffer byte-wise from pBuf. Address must be the first entries in
     * pBuf
     */
    uint8_t *data = (uint8_t *)pBuf;

    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_CLR);
    for (unsigned int i = 0; i < n; i++)
    {
        sercom->SPI.DATA.reg = *data++;
        while(0 == (sercom->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC));
    }
    portPinDrv(GRP_SERCOM_SPI, PIN_SPI_RFM_SS, PIN_DRV_SET);
}
