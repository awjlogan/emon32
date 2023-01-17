#include "emon32_samd.h"

static unsigned int dmaInitFlag;

/*! @brief Get the DMAC descriptor, and set up initial values */
static void
dmaInit()
{
    volatile DmacDescriptor * dmacDesc = dmacGetDescriptor(DMA_CHAN_I2CM);
    dmacDesc->BTCTRL.reg =   DMAC_BTCTRL_VALID
                           | DMAC_BTCTRL_BLOCKACT_NOACT
                           | DMAC_BTCTRL_STEPSIZE_X1
                           | DMAC_BTCTRL_STEPSEL_SRC
                           | DMAC_BTCTRL_SRCINC
                           | DMAC_BTCTRL_BEATSIZE_BYTE;

    dmacDesc->DSTADDR.reg = (uint32_t)&SERCOM_UART_DBG->USART.DATA;
    dmacDesc->DESCADDR.reg = 0u;
    dmaInitFlag = 1u;
}

void
eepromWrite(unsigned int addr, const void *pSrc, unsigned int n)
{
    #ifndef EEPROM_EMULATED

    uint8_t *pData = (uint8_t *)pSrc;
    const DMACCfgCh_t ctrlb_i2c_wr = {.ctrlb =   DMAC_CHCTRLB_LVL(1u)
                                               | DMAC_CHCTRLB_TRIGACT_BEAT
                                               | DMAC_CHCTRLB_TRIGSRC(SERCOM_I2CM_DMAC_ID_TX)};
    volatile DmacDescriptor *dmacDesc = dmacGetDescriptor(DMA_CHAN_I2CM);

    if (0 == dmaInitFlag)
    {
        dmaInit();
    }

    dmacDesc->BTCTRL.reg =   DMAC_BTCTRL_BLOCKACT_NOACT
                           | DMAC_BTCTRL_STEPSIZE_X1
                           | DMAC_BTCTRL_STEPSEL_SRC
                           | DMAC_BTCTRL_SRCINC
                           | DMAC_BTCTRL_BEATSIZE_BYTE;
    dmacDesc->DSTADDR.reg = (uint32_t)&SERCOM_I2CM->I2CM.DATA;
    dmacDesc->DESCADDR.reg = 0u;

    dmacChannelConfigure(DMA_CHAN_I2CM, &ctrlb_i2c_wr);

    /* Write first contiguous sequence */
    while (n > EEPROM_PAGE_SIZE)
    {
        dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
        dmacDesc->BTCNT.reg = EEPROM_PAGE_SIZE;
        dmacDesc->SRCADDR.reg = (uint32_t)(pData + EEPROM_PAGE_SIZE);
        dmacStartTransfer(DMA_CHAN_I2CM);

        i2cActivate(SERCOM_I2CM, addr, 1u, EEPROM_PAGE_SIZE);
        n -= EEPROM_PAGE_SIZE;
        addr += EEPROM_PAGE_SIZE;
        pData += EEPROM_PAGE_SIZE;
    }

    /* Mop up residual data */
    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = n;
    dmacDesc->SRCADDR.reg = (uint32_t)(pData + n);
    dmacStartTransfer(DMA_CHAN_I2CM);
    i2cActivate(SERCOM_I2CM, addr, 1u, n);

    #endif
}

void
eepromRead(unsigned int addr, void *pDst, unsigned int n)
{
    #ifndef EEPROM_EMULATED

    uint8_t *pData = (uint8_t *)pDst;
    const DMACCfgCh_t ctrlb_i2c_wr = {.ctrlb =   DMAC_CHCTRLB_LVL(1u)
                                               | DMAC_CHCTRLB_TRIGACT_BEAT
                                               | DMAC_CHCTRLB_TRIGSRC(SERCOM_I2CM_DMAC_ID_RX)};

    volatile DmacDescriptor *dmacDesc = dmacGetDescriptor(DMA_CHAN_I2CM);

    if (0 == dmaInitFlag)
    {
        dmaInit();
    }

    dmacDesc->BTCTRL.reg =   DMAC_BTCTRL_BLOCKACT_NOACT
                           | DMAC_BTCTRL_STEPSIZE_X1
                           | DMAC_BTCTRL_STEPSEL_DST
                           | DMAC_BTCTRL_DSTINC
                           | DMAC_BTCTRL_BEATSIZE_BYTE;
    dmacDesc->SRCADDR.reg = (uint32_t)&SERCOM_I2CM->I2CM.DATA;
    dmacDesc->DESCADDR.reg = 0u;

    dmacChannelConfigure(DMA_CHAN_I2CM, &ctrlb_i2c_wr);

    /* Write first contiguous sequence */
    while (n > EEPROM_PAGE_SIZE)
    {
        dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
        dmacDesc->BTCNT.reg = EEPROM_PAGE_SIZE;
        dmacDesc->DSTADDR.reg = (uint32_t)pData;
        dmacStartTransfer(DMA_CHAN_I2CM);

        i2cActivate(SERCOM_I2CM, addr, 1u, EEPROM_PAGE_SIZE);
        n -= EEPROM_PAGE_SIZE;
        addr += EEPROM_PAGE_SIZE;
        pData += EEPROM_PAGE_SIZE;
    }

    /* Mop up residual data */
    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = n;
    dmacDesc->DSTADDR.reg = (uint32_t)&SERCOM_I2CM->I2CM.DATA;
    dmacStartTransfer(DMA_CHAN_I2CM);
    i2cActivate(SERCOM_I2CM, addr, 1u, n);

    #endif /* EEPROM_EMULATED */
}

/*! @brief Find the index of the last valid write to a wear levelled block
 *  @param [in] pPkt : pointer to the packet
 */
void
wlFindLast(eepromPktWL_t *pPkt)
{
    /* Step through from the base address is (data) sized steps. The first
     * byte that is different to the 0-th byte is the stalest block. If all
     * blocks are the same, then the 0-th index is the next to be written
     */

    uint8_t idxLastWr = 0;
    uint8_t firstByte = 0;

    firstByte = eepromRead(pPkt->addr_base, &firstByte, 1u);
    for (unsigned int idxBlk = 1u; idxBlk < pPkt->blkCnt; idxBlk++)
    {
        uint8_t validByte;
        unsigned int addrRd = pPkt->addr_base + (pPkt->dataSize * idxBlk);
        validByte = eepromRead(addrRd, &validByte, 1u);
        idxLastWr++;

        if (firstByte != validByte)
        {
            pPkt->idxLastWr = idxLastWr
        }
    }
}
