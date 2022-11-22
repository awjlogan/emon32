#include "emon32_samd.h"

void
adcSetup()
{
    /* APB bus clock is enabled by default (Table 15-1). Connect GCLK 3 */
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(ADC_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Calibration values - Table 9-4. Linearity split over 2 32bit words */
    const uint32_t calADCbias = (CAL_REG_HIGH & CAL_ADC_BIAS_Msk) >> CAL_ADC_BIAS_Pos;
    const uint32_t calADClin =   ((CAL_REG_LOW & CAL_ADC_LIN_L_Msk) >> CAL_ADC_LIN_L_Pos)
                               | (CAL_REG_HIGH & CAL_ADC_LIN_H_Msk) << 5u;

    ADC->CALIB.reg =   (uint16_t)calADCbias << 8u
                     | (uint16_t)calADClin;

    /* Enable reference buffer and set to external VREF
     * TODO Unclear if the buffer is for external ref, or only internal
     */
    ADC->REFCTRL.reg =   ADC_REFCTRL_REFCOMP
                       | ADC_REFCTRL_REFSEL_AREFA;

    /* Differential mode, /8 prescale of F_PERIPH  */
    ADC->CTRLB.reg =   ADC_CTRLB_PRESCALER_DIV8
                     | ADC_CTRLB_DIFFMODE;
    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY);

    /* Setup 15 us conversion time - allows up to ~65 ksps @ 1 MHz CLK
     * SAMPLEN = (2T / T_clk) - 1 = 29
     */
    ADC->SAMPCTRL.reg = 0x1Du;

    /* Input control */
    ADC->INPUTCTRL.reg =   ADC_INPUTCTRL_MUXPOS_PIN2
                         | ADC_INPUTCTRL_MUXNEG_PIN0
                         /* INPUTSCAN is number of channels - 1 */
                         | ADC_INPUTCTRL_INPUTSCAN(NUM_V + NUM_CT - 1u);

    /* ADC is triggered by an event from TC1 with no CPU intervention */
    ADC->EVCTRL.reg = ADC_EVCTRL_STARTEI;

    /* DMA channel */
    volatile DmacDescriptor *dmacDesc = dmacGetDescriptor(DMA_CHAN_ADC);
    dmacDesc->DESCADDR.reg = 0u;
    dmacDesc->SRCADDR.reg = (uint32_t)&ADC->RESULT;
    /* Likely to downsample / 2, so capture two sample sets before interrupt */
    dmacDesc->BTCNT.reg = (NUM_V + NUM_CT) * 2u;
    dmacDesc->BTCTRL.reg =   DMAC_BTCTRL_VALID
                           | DMAC_BTCTRL_BLOCKACT_NOACT
                           | DMAC_BTCTRL_BEATSIZE_HWORD
                           | DMAC_BTCTRL_DSTINC
                           | DMAC_BTCTRL_STEPSIZE_X1;

    dmacEnableChannelInterrupt(DMA_CHAN_ADC);
}

void
adcSetDestination(uint32_t buf)
{
    volatile DmacDescriptor *dmaDesc = dmacGetDescriptor(DMA_CHAN_ADC);
    dmaDesc->DSTADDR.reg = buf;
}
