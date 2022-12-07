#include "emon32_samd.h"

/* Event handlers */
static volatile uint32_t evtPend;

/*! @brief Indicate a pending event. Interrupts are disabled briefly to allow
 *         the RMW update to complete safely.
 */
void
emon32SetEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = (1u << evt);
    __disable_irq();
    evtPend |= evtDecode;
    __enable_irq();
}


/*! @brief Clear a pending event. Interrupts are disabled briefly to allow the
 *         RMW update to complete safely.
 */
void
emon32ClrEvent(INTSRC_t evt)
{
    /* Disable interrupts during RMW update of event status */
    uint32_t evtDecode = ~(1u << evt);
    __disable_irq();
    evtPend &= evtDecode;
    __enable_irq();
}

/*! @brief This function is called when the 1 ms timer overflows (SYSTICK).
 *         Latency is not guaranteed, so only non-timing critical things
 *         should be done here (UI update, watchdog etc)
 */
void
evtKiloHertz()
{
    static unsigned int khz_ticks;
    khz_ticks++;
    (void)uiSWUpdate();
    uiUpdateLED(EMON_IDLE);

    /* Kick watchdog - placed in the event handler to allow reset of stuck
     * processing rather than entering the interrupt reliably
     */
    wdtKick();

    if (500 == khz_ticks)
    {
        uartPutcBlocking(SERCOM_UART_DBG, '*');
        khz_ticks = 0u;
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
    wdtSetup(WDT_PER_4K);
};

int
main()
{
    /* Configuration */
    Emon32Config_t e32Config;

    /* Processed data */
    ECMSet_t dataset;

    /* TODO check size of buffer */
    char txBuffer[64];

    setup_uc();

    /* Setup DMAC for non-blocking UART (this is optional, unlike ADC) */
    uartConfigureDMA();
    uartPutsBlocking(SERCOM_UART_DBG, "\r\n== Energy Monitor 32 ==\r\n");
    adcStartDMAC((uint32_t)ecmDataBuffer());

    /* Indicate if the reset syndrome was from the watchdog */
    if (PM->RCAUSE.reg & PM_RCAUSE_WDT)
    {
        uartPutsNonBlocking(DMA_CHAN_UART_DBG, "\r\n> Reset by WDT\r\n", 18u);
    }

    /* If the button is pressed at reset, enter configuration mode
     * Allow 50 ms roll overs to ensure the button has been debounced.
     * The configuration is saved to EEPROM, and the uc is then reset.
     */
    unsigned int systickCnt = 0u;
    while (systickCnt < 50)
    {
        if (evtPend & (1u << EVT_SYSTICK_1KHz))
        {
            evtKiloHertz();
            systickCnt++;
        }
    }
    if (SW_PRESSED == uiSWState())
    {
//         configEnter(&e32Config);
    }

    /* TODO Load configuration from EEPROM */

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
                /* Generate the report, pack, and send */
                unsigned int pktLength;
                ecmProcessSet(&dataset);
                pktLength = emon32PackageData(&dataset, txBuffer);
                uartPutsNonBlocking(DMA_CHAN_UART_DBG, txBuffer, pktLength);
                emon32ClrEvent(EVT_ECM_SET_CMPL);
            }
        }
        __WFI();
    };
}
