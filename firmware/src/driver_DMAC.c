#include "emon32_samd.h"

static volatile DmacDescriptor dmacs[NUM_CHAN_DMA];
static DmacDescriptor dmacs_wb[NUM_CHAN_DMA];

/* Useful ref: https://aykevl.nl/2019/09/samd21-dma */

void
dmacSetup()
{
    /* Clocking - AHB and APB are both enabled at reset (15.8.8, 15.8.10) */
    DMAC->BASEADDR.reg = (uint32_t)dmacs;
    DMAC->WRBADDR.reg = (uint32_t)dmacs_wb;
    DMAC->CTRL.reg =   DMAC_CTRL_DMAENABLE
                     | DMAC_CTRL_LVLEN(0xFu);

    /* UART DMA */
    DMAC->CHID.reg = DMA_CHAN_UART;
    DMAC->CHCTRLB.reg =   DMAC_CHCTRLB_LVL(1u)
                        | DMAC_CHCTRLB_TRIGSRC(SERCOM0_DMAC_ID_TX)
                        | DMAC_CHCTRLB_TRIGACT_BEAT;
}

volatile DmacDescriptor *
dmacGetDescriptor(unsigned int ch)
{
    return &dmacs[ch];
}

void
dmacStartTransfer(unsigned int ch)
{
    DMAC->CHID.reg = (uint8_t)ch;
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}
