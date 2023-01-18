#include "board_def.h"
#include "driver_DMAC.h"
#include "driver_SERCOM.h"
#include "eeprom.h"
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

    dmacDesc->DSTADDR.reg = (uint32_t)&SERCOM_I2CM->I2CM.DATA;
    dmacDesc->DESCADDR.reg = 0u;
    dmaInitFlag = 1u;
}

void
eepromWrite(unsigned int addr, const void *pSrc, unsigned int n)
{
    #ifndef EEPROM_EMULATED

    uint8_t align_bytes = 16u - (addr & 0xFu);
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

    /* For 24C0x EEPROMs, writes can not go over 16 byte pages. Align the
     * first transfer to end of 16 byte page.
     */
    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = (align_bytes > n) ? n : align_bytes;
    dmacDesc->SRCADDR.reg = (uint32_t)(pData);
    dmacStartTransfer(DMA_CHAN_I2CM);
    i2cActivate(SERCOM_I2CM, addr, 1u, n);
    if (align_bytes > n)
    {
        n = 0;
    }
    else
    {
        n -= align_bytes;
        pData += align_bytes;
    }

    /* Write any contiguous sequence */
    while (n > EEPROM_PAGE_SIZE)
    {
        dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
        dmacDesc->BTCNT.reg = EEPROM_PAGE_SIZE;
        dmacDesc->SRCADDR.reg = (uint32_t)pData;
        dmacStartTransfer(DMA_CHAN_I2CM);

        i2cActivate(SERCOM_I2CM, addr, 1u, EEPROM_PAGE_SIZE);
        n -= EEPROM_PAGE_SIZE;
        addr += EEPROM_PAGE_SIZE;
        pData += EEPROM_PAGE_SIZE;
    }

    /* Mop up residual data */
    if (0 != n)
    {
        dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
        dmacDesc->BTCNT.reg = n;
        dmacDesc->SRCADDR.reg = (uint32_t)pData;
        dmacStartTransfer(DMA_CHAN_I2CM);
        i2cActivate(SERCOM_I2CM, addr, 1u, n);
    }

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

    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = n;
    dmacDesc->DSTADDR.reg = (uint32_t)pData;
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

    uint8_t idxNextWr = 0;
    uint8_t firstByte = 0;

    eepromRead(pPkt->addr_base, &firstByte, 1u);
    for (unsigned int idxBlk = 1u; idxBlk < pPkt->blkCnt; idxBlk++)
    {
        uint8_t validByte;
        unsigned int addrRd = pPkt->addr_base + (pPkt->dataSize * idxBlk);
        eepromRead(addrRd, &validByte, 1u);
        idxNextWr++;

        if (firstByte != validByte)
        {
            break;
        }
    }

    pPkt->idxNextWrite = (idxNextWr == (pPkt->blkCnt - 1u)) ? 0 : idxNextWr;
}

void
eepromWriteWL(eepromPktWL_t *pPktWr)
{
    /* Check for correct indexing, find if not yet set. Write output to new
     * levelled position, and increment index.
     */
    uint8_t     validByte;
    int8_t      idxWr;
    uint16_t    addrWr;

    if (-1 == pPktWr->idxNextWrite) wlFindLast(pPktWr);

    /* Calculate the next valid byte, and store packet */
    addrWr = pPktWr->addr_base + (pPktWr->idxNextWrite * pPktWr->dataSize);
    eepromRead((addrWr - pPktWr->dataSize), &validByte, 1u);

    /* The valid byte is calculated to have even bit 0/1 writes. */
    if (0 == validByte)
    {
        validByte += 1u;
    }
    else if (0xFFu == validByte)
    {
        validByte <<= 1;
    }
    else if (1u == (validByte & 0x1u))
    {
        validByte <<= 1;
        validByte += 1u;
    }
    else
    {
        validByte <<= 1;
    }
    *(uint8_t *)pPktWr->pData = validByte;

    eepromWrite(addrWr, pPktWr->pData, pPktWr->dataSize);

    idxWr = pPktWr->idxNextWrite + 1u;
    if (idxWr == pPktWr->blkCnt)
    {
        idxWr = 0;
    }
    pPktWr->idxNextWrite = idxWr;
}

void
eepromReadWL(eepromPktWL_t *pPktRd)
{
    /* Check for correct indexing, find it not yet set. Read into struct from
     * correct location.
     */
    int8_t idxRd;
    uint16_t addrRd;

    if (-1 == pPktRd->idxNextWrite) wlFindLast(pPktRd);
    idxRd = pPktRd->idxNextWrite - 1u;
    if (idxRd < 0)
    {
        idxRd = pPktRd->blkCnt + idxRd;
    }
    addrRd = pPktRd->addr_base + (idxRd * pPktRd->dataSize);
    eepromRead(addrRd, pPktRd->pData, pPktRd->dataSize);
}
