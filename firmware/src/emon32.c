#include "driver_DMAC.h"
#include "emon32_samd.h"

static uint32_t evtPend;

void
emon32SetEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = (1u << (uint32_t)evt);
    __disable_irq();
    evtPend |= evtDecode;
    __enable_irq();
}

void
emon32ClrEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = ~(1u << (uint32_t)evt);
    __disable_irq();
    evtPend &= evtDecode;
    __enable_irq();
}

void
evtKiloHertz()
{
    static unsigned int khz_ticks;
    khz_ticks++;
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
    setup_uc();

    /* Setup DMAC for non-blocking UART */
    uartConfigureDMA();

    const char welcome[] = "\r\n== Energy Monitor 32 ==\r\n";
    uartPutsBlocking(welcome);

    while(0 != evtPend)
    {
        if (evtPend & (1u << EVT_SYSTICK_1KHz))
        {
            evtKiloHertz();
        }
        __WFI();
    };
}
