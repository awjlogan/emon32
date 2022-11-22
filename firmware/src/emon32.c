#include "emon32_samd.h"
#include "emon_CM.h"

/* Event handlers */
static uint32_t evtPend;

void
emon32SetEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = (1u << evt);
    __disable_irq();
    evtPend |= evtDecode;
    __enable_irq();
}

void
emon32ClrEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = ~(1u << evt);
    __disable_irq();
    evtPend &= evtDecode;
    __enable_irq();
}

void
evtKiloHertz()
{
    static unsigned int khz_ticks;
    khz_ticks++;
    (void)uiUpdateSW();
    uiUpdateLED(EMON_IDLE);
    if (500 == khz_ticks)
    {
        khz_ticks = 0u;
        uartPutcBlocking('.');
    }
    emon32ClrEvent(EVT_SYSTICK_1KHz);
}


/* @brief This function mustbe called first. An implementation must provide
 *        all the functions that are called; these can be empty if they are
 *        not used.
 */
static inline void
setup_uc()
{
    clkSetup();
    timerSetup();
    portSetup();
    sercomSetup();
    adcSetup();
    dmacSetup();
    evsysSetup();
};

int
main()
{
    /* Sample storage */
    volatile SampleSetPacked_t samples[SAMPLE_BUF_DEPTH];
    volatile SampleSetPacked_t *volatile smp_active = &samples[0];
    volatile SampleSetPacked_t *volatile smp_proc = &samples[1];
    SampleSet_t smp_inject;

    setup_uc();

    /* Setup DMAC for non-blocking UART (this is optional, unlike ADC) */
    uartConfigureDMA();
    uartPutsBlocking("\r\n== Energy Monitor 32 ==\r\n");

    /* Set ADC DMA destination */
    adcSetDestination((uint32_t)smp_active);

    while(0 != evtPend)
    {
        if (evtPend & (1u << EVT_SYSTICK_1KHz))
        {
            evtKiloHertz();
        }
        if (evtPend & (1u << EVT_DMAC_SMP_CMPL))
        {
            preUnpackSample(smp_active, &smp_inject);
        }
        __WFI();
    };
}
