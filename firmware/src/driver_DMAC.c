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

    /* TODO Make the init for ADC and DMA independent from the DMAC driver */
    /* UART DMA */
    DMAC->CHID.reg = DMA_CHAN_UART_DBG;
    DMAC->CHCTRLB.reg =   DMAC_CHCTRLB_LVL(1u)
                        | DMAC_CHCTRLB_TRIGSRC(SERCOM_UART_DBG_DMAC_ID_TX)
                        | DMAC_CHCTRLB_TRIGACT_BEAT;


    /* ADC DMA */
    DMAC->CHID.reg = DMA_CHAN_ADC;
    DMAC->CHCTRLB.reg =   DMAC_CHCTRLB_LVL(0u)
                        | DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY)
                        | DMAC_CHCTRLB_TRIGACT_BEAT;

    /* Enable the DMAC interrupt in the NVIC, but leave the channel interrupt
     * enable/disable for each channel to the peripheral */
    NVIC_EnableIRQ(DMAC_IRQn);
}

volatile DmacDescriptor *
dmacGetDescriptor(unsigned int ch)
{
    return &dmacs[ch];
}

void
dmacStartTransfer(unsigned int ch)
{
    DMAC->CHID.reg = ch;
    DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

void
dmacEnableChannelInterrupt(unsigned int ch)
{
    DMAC->CHID.reg = ch;
    DMAC->CHINTENSET.reg |= DMAC_CHINTENSET_TCMPL;
}

void
dmacDisableChannelInterrupt(unsigned int ch)
{
    DMAC->CHID.reg = ch;
    DMAC->CHINTENCLR.reg |= DMAC_CHINTENCLR_TCMPL;
}

void
dmacClearChannelInterrupt(unsigned int ch)
{
    DMAC->CHID.reg = ch;
    DMAC->CHINTFLAG.reg |= DMAC_CHINTFLAG_TCMPL;
}

void
irq_handler_dmac()
{
    /* Check which channel has triggered the interrupt, set the event, and
     * clear the interrupt source
     */
    DMAC->CHID.reg = DMA_CHAN_ADC;
    if (DMAC->CHINTFLAG.reg & DMAC_CHINTFLAG_TCMPL)
    {
        /* Restart DMA for ADC here, raise flag to handle sample */
        ecmSwapDataBuffer();
        adcStartDMAC((uint32_t)ecmDataBuffer());
        emon32SetEvent(EVT_DMAC_SMP_CMPL);
        DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
    }

    DMAC->CHID.reg = DMA_CHAN_UART_DBG;
    if (DMAC->CHINTFLAG.reg & DMAC_CHINTFLAG_TCMPL)
    {
        emon32SetEvent(EVT_DMAC_UART_CMPL);
        DMAC->CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
    }
}
