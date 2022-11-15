#include "emon32_samd.h"

void
adcSetup()
{
    /* APB bus clock is enabled by default (Table 15-1). Connect GCLK 3 */
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(ADC_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Enable reference buffer and set to external VREF
     * TODO Unclear if the buffer is for external ref, or only internal
     */
    ADC->REFCTRL.reg =   ADC_REFCTRL_REFCOMP
                       | ADC_REFCTRL_REFSEL_AREFA;

    /* Free running conversion, differential mode */
    ADC->CTRLB.reg =   ADC_CTRLB_DIFFMODE
                     | ADC_CTRLB_FREERUN;
    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY);



}
