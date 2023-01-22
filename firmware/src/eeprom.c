#include <string.h>

#include "emon32_samd.h"

/* A write including the lower address byte */
typedef struct __attribute__((__packed__)) {
    uint8_t         addrLow;
    uint8_t         data[EEPROM_PAGE_SIZE];
} writePkt_t;

/* Local data for interrupt driven EEPROM write */
typedef struct {
    uint16_t        addr;
    unsigned int    n;
    writePkt        *pPkt;
    uint8_t         *pData;
} wrLocal_t;

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

    dmacEnableChannelInterrupt(DMA_CHAN_I2CM);

    dmaInitFlag = 1u;
}

/*! @brief Send n bytes over I2C using DMA
 *  @param [in] dmacDesc : pointer to DMAC descriptor struct
 *  @param [in] wr : pointer to local address, data, and remaining bytes
 *  @param [in] n : number of bytes to send in this chunk
 */
static void
writeSetup(DmacDescriptor *dmacDesc, wrLocal_t *wr, uint8_t n)
{
    /* Construct the address bytes. Top half of the EEPROM address is in the
       device select packet. Bottom half is the first byte sent */
    uint8_t i2cBase = EEPROM_BASE_ADDR;
    i2cBase |= (wr->addr >> 8);
    i2cBase <<= 1u;
    wr->pPkt = (wr->addr & 0xFFu);

    /* Copy data from the source into the write buffer */
    memcpy((wr->wrPkt + 1u), wr->pData, n)

    /* Need to send an extra byte (n + 1) for the low address */
    dmacDesc->SRCADDR.reg = wr->pPkt + (n + 1u);
    dmacDesc->BTCTRL.reg |= DMAC_BTCTRL_VALID;
    dmacDesc->BTCNT.reg = n + 1u;
    dmacStartTransfer(DMA_CHAN_I2CM);
    i2cActivate(SERCOM_I2CM, i2cBase, 1u, (n + 1u));

    wr->addr += n;
    wr->n -= n;
    wr->pData += n;
}

eepromWrStatus_t
eepromWrite(unsigned int addr, const void *pSrc, unsigned int n)
{
    #ifndef EEPROM_EMULATED

    writePkt_t          wrPkt;
    /* Make byte count and address static to allow re-entrant writes */
    static wrLocal_t    wrLocal;
    uint8_t             align_bytes;

    /* If all parameters are 0, then this is a continuation from ISR */
    const unsigned int  continueBlock = (0 == addr) && (0 == pSrc) && (0 == n);

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

    /* If no ongoing transaction:
     *   - if it is a continuation, then return complete
     *   - otherwise capture required details
     */
    if (0 == wrLocal.n)
    {
        if (0 != continueBlock)
        {
            return EEPROM_WR_COMPLETE;
        }

        pData = (uint8_t *)pSrc;
        wrLocal.n = n;
        wrLocal.pPkt = &wrPkt;
        wrLocal.pData = (uint8_t *)pSrc;
        wrLocal.addr = addr;
    }
    /* If there is a pending data, and this is new, indicate BUSY */
    else
    {
        if (0 == continueBlock)
        {
            return EEPROM_WR_BUSY;
        }
        else
        {
            if (0 != dmacChannelBusy(DMA_CHAN_I2CM))
            {
                return EEPROM_WR_BUSY;
            }
        }
    }

    /* Writes can not go over EEPROM_PAGE_SIZE byte pages. Align the first
     * transfer to end of EEPROM_PAGE_SIZE byte page.
     */
    align_bytes = (EEPROM_PAGE_SIZE - (wrLocal.addr & (EEPROM_PAGE_SIZE - 1u))) % EEPROM_PAGE_SIZE;
    if (0 != align_bytes)
    {
        if (align_bytes > n)
        {
                align_bytes = n;
        }

        /* Copy data into the write packet's data section */
        writeSetup(dmacDesc, &wrLocal, align_bytes);

        return EEPROM_WR_PEND;
    }

    /* Write any whole pages */
    while (n > EEPROM_PAGE_SIZE)
    {
        writeSetup(dmacDesc, &wrLocal, EEPROM_PAGE_SIZE);
        return EEPROM_WR_PEND;
    }

    /* Mop up residual data */
    writeSetup(dmacDesc, &wrLocal, wrLocal.n);
    return EEPROM_WR_PEND;

    #endif /* EEPROM_EMULATED */
}

void
eepromRead(unsigned int addr, void *pDst, unsigned int n)
{
    #ifndef EEPROM_EMULATED

    uint8_t addrHigh;
    uint8_t addrLow;

    uint8_t *pData = (uint8_t *)pDst;
    const DMACCfgCh_t ctrlb_i2c_wr = {.ctrlb =   DMAC_CHCTRLB_LVL(1u)
                                               | DMAC_CHCTRLB_TRIGACT_BEAT
                                               | DMAC_CHCTRLB_TRIGSRC(SERCOM_I2CM_DMAC_ID_RX)};

    volatile DmacDescriptor *dmacDesc = dmacGetDescriptor(DMA_CHAN_I2CM);

    /* Set the addres to read from in the EEPROM. This is a select write,
     * followed by the byte low address. */
    addrHigh =   EEPROM_BASE_ADDR
               | (addr >> 8);
    addrHigh <<= 1u;
    addrLow = addr & 0xFFu;

    /* Send address, wait for ack, then send low byte of address */
    /* TODO handle timeouts and other errors gracefully */
    i2cActivate(SERCOM_I2CM, addrHigh, 0u, 0u);
    while (!(SERCOM_I2CM->INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

    if (!(SERCOM_I2CM->STATUS.reg & SERCOM_I2CM_STATUS_RXNACK))
    {
        SERCOM_I2CM->DATA.reg = addrLow;
    }

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

    /* Now ready to read sequentially, issue Select packet, and DMA will fill
     * the pData buffer. The DMAC will raise an interrupt when complete */
    addrHigh = (EEPROM_BASE_ADDR << 1u) + 1u;
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

    /* TODO Compare checksum of packet to ensure that it is not corrupted */
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
    /* Start filling with 1s */
    if (0 == validByte)
    {
        validByte += 1u;
    }
    /* Start filling with 0s */
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

    (void)eepromWrite(addrWr, pPktWr->pData, pPktWr->dataSize);

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
    int8_t      idxRd;
    uint16_t    addrRd;

    if (-1 == pPktRd->idxNextWrite) wlFindLast(pPktRd);
    idxRd = pPktRd->idxNextWrite - 1u;
    if (idxRd < 0)
    {
        idxRd = pPktRd->blkCnt + idxRd;
    }
    addrRd = pPktRd->addr_base + (idxRd * pPktRd->dataSize);
    eepromRead(addrRd, pPktRd->pData, pPktRd->dataSize);
}
