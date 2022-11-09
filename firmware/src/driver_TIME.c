#include "emon32_samd.h"

void
timer_setup()
{
    /* Enable APB clock, set TC1 to generator 3 (OSC8M @ F_PERIPH)  */
    PM->APBCMASK.reg |= PM_APBCMASK_TC1;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(TC1_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;
    /* Configure TC1 as 32bit
}

void
timer_delay_ms(uint16_t delay)
{
    timer_delay_us(delay * 1000);
}

void
timer_delay_us(uint32_t delay)
{
    /* TODO Pause interrupts */
    /* Get current time, add delay then wait */
    const uint32_t delayUntil = delay;
    while (delayUntil > TC1);
}

/* Interrupt handler */
