#include "emon32_samd.h"

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

/*! @brief This function must be called first. An implementation must provide
 *         all the functions that are called; these can be empty if they are
 *         not used.
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
    /* Processed data */
    ECMSet_t dataset;
    ECMSet_t *const pDataset = &dataset;

    setup_uc();

    /* Setup DMAC for non-blocking UART (this is optional, unlike ADC) */
    uartConfigureDMA();
    uartPutsBlocking("\r\n== Energy Monitor 32 ==\r\n");

    for (;;)
    {
        /* While there is an event pending (may be set while another is
         * handled, keep looping. Enter sleep (WFI) when done.
         */
        while(0 != evtPend)
        {
            if (evtPend & (1u << EVT_SYSTICK_1KHz))
            {
                evtKiloHertz();
            }
            if (evtPend & (1u << EVT_DMAC_SMP_CMPL))
            {
                /* TODO Check timings; needs to be serviced within 1/2400 s */
                ecmInjectSample();
                emon32ClrEvent(EVT_DMAC_SMP_CMPL);
            }
            if (evtPend & (1u << EVT_ECM_CYCLE_CMPL))
            {
                ecmProcessCycle();
                emon32ClrEvent(EVT_ECM_CYCLE_CMPL);
            }
            if (evtPend & (1u << EVT_ECM_SET_CMPL))
            {
                ecmProcessSet(pDataset);
                emon32ClrEvent(EVT_ECM_SET_CMPL);
            }
        }
        /* TODO Enter WFI when done, get working first before
         * introducing any sleep modes!
         * __WFI();
         */
    };
}
