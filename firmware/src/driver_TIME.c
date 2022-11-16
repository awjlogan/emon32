#include "emon32_samd.h"

void
timerSetup()
{
    /* SysTick is used as the general purpose 1 kHz timer
     * The tick value is reloaded on underflow
     * SysTick is part of CMSIS so should be portable across Cortex-M cores
     */
    const uint32_t tickkHz = (F_CPU / 1000u) - 1u;
    SysTick_Config(tickkHz);

    /* TC1 is used to trigger ADC sampling at constant rate */
    /* Enable APB clock, set TC1 to generator 3 (OSC8M @ F_PERIPH)  */
    PM->APBCMASK.reg |= PM_APBCMASK_TC1;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(TC1_GCLK_ID)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure as 8bit counter (F_PERIPH - /8 -> F_TC1 */
    TC1->COUNT8.CTRLA.reg =   TC_CTRLA_MODE_COUNT8
                            | TC_CTRLA_PRESCALER_DIV8
                            | TC_CTRLA_PRESCSYNC_RESYNC;

    /* TC1 overflow event output to trigger ADC */
    TC1->COUNT8.EVCTRL.reg = TC_EVCTRL_OVFEO;

    /* TC1 is running at 1 MHz, each tick is 1 us */
    const unsigned int cntPer = F_TC1 / SAMPLE_RATE / (NUM_V + NUM_CT);
    TC1->COUNT8.PER.reg = (uint8_t)cntPer;
    TC1->COUNT8.COUNT.reg = 0u;
}

void
timerDelay_ms(uint16_t delay)
{
    timerDelay_us(delay * 1000u);
}

void
timerDelay_us(uint32_t delay)
{
    /* Get current time, add delay then wait
     * TODO Catch wrap around overflow of counter
     */
    const uint32_t delayUntil = delay + TC1->COUNT32.COUNT.reg;
    while (delayUntil > TC1->COUNT32.COUNT.reg);
}

void
timerADCTriggerRate(uint16_t sample_rate)
{
    /* The ADC is triggered using an event at a given sample rate
     * This is caused by TC2 overflowing at the TOP value, so calculate
     * the value for TOP by:
     * TOP = 1/sample_rate * 1/(NUM_V + NUM_CT)
     * TODO scale top for frequency other than 1 MHz on the counter
     * TODO should this just be a define to avoid the unneeded division?
     */
    const uint16_t top = 1.0/sample_rate * 1.0 / (NUM_V + NUM_CT) * 1E6;
    TC2->COUNT16.CC[0].reg = top;
}

void
irq_handler_tc1()
{
}

void
irq_handler_sys_tick()
{
    emon32SetEvent(EVT_SYSTICK_1KHz);
}
