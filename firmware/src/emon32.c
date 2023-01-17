#include "emon32_samd.h"

/* Persistent state variables */
static volatile uint32_t    evtPend;
static volatile EmonState_t emonState = EMON_STATE_IDLE;

/*! @brief The default configuration state of the system */
static inline void
defaultConfiguration(Emon32Config_t *pCfg, Emon32Cumulative_t *pRes)
{
    /* Default configuration: single phase, 50 Hz, 240 VAC */
    pCfg->baseCfg.mainsFreq = 50u;      /* Mains frequency */
    pCfg->baseCfg.reportCycles = 500u;  /* 10 s @ 50 Hz */
    pCfg->baseCfg.equilCycles = 5u;     /* Warm up cycles to populate buffers */

    for (unsigned int idxV = 0u; idxV < NUM_V; idxV++)
    {
        pCfg->voltageCfg[idxV].voltageCal = 268.97;
    }

    /* 4.2 degree shift @ 50 Hz, 4 CTs */
    for (unsigned int idxCT = 0u; idxCT < NUM_CT; idxCT++)
    {
        pCfg->ctCfg[idxCT].ctCal = 90.91;
        pCfg->ctCfg[idxCT].phaseX = 13495;
        pCfg->ctCfg[idxCT].phaseY = 19340;
        pRes->wattHour[idxCT] = 0;
    }
    pRes->pulseCnt = 0;
}

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
emon32StateSet(EmonState_t state)
{
    emonState = state;
}

EmonState_t
emon32StateGet()
{
    return emonState;
}

/*! @brief This function handles loading of configuration data
 *  @param [in] pCfg : pointer to the configuration struct
 */
static inline void
loadConfiguration(Emon32Config_t *pCfg, Emon32Cumulative_t *pRes)
{
    unsigned int systickCnt = 0u;
    unsigned int seconds = 3u;
    uint8_t      eepromByte;

    /* Load first byte from EEPROM - if 0, the EEPROM has been programmed and
     * the configuration data should be loaded from that.
     */
//     eepromRead(EEPROM_BASE_ADDR, (void *)&eepromByte, 1u);
//     if (0 == eepromByte)
//     {
//         eepromRead((EEPROM_BASE_ADDR + EEPROM_PAGE_SIZE), (void *)pCfg, sizeof(Emon32Config_t));
//         /* TODO Read residual values from last start */
//     }
//     else
//     {
//         defaultConfiguration(pCfg, pRes);
//     }

    /* Wait for 3 s, if a key is pressed then enter configuration */
    uartPutsBlocking(SERCOM_UART_DBG, "\r\n> Hit any key to enter configuration ");
    while (systickCnt < 4095)
    {
        if (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC)
        {
            emon32StateSet(EMON_STATE_CONFIG);
            (void)uartGetc(SERCOM_UART_DBG);
            configEnter(pCfg);
            break;
        }

        if (evtPend & (1u << EVT_SYSTICK_1KHz))
        {
            wdtKick();
            emon32ClrEvent(EVT_SYSTICK_1KHz);
            systickCnt++;

            /* Countdown every second, tick every 1/4 second to debug UART */
            if (0 == (systickCnt & 0x3FF))
            {
                uartPutcBlocking(SERCOM_UART_DBG, '0' + seconds);
                seconds--;
            }
            else if (0 == (systickCnt & 0xFF))
            {
                uartPutcBlocking(SERCOM_UART_DBG, '.');
            }
        }
    }
    uartPutsBlocking(SERCOM_UART_DBG, "\r\n> Configuration loaded, start sampling!\r\n\r\n");
}

/*! @brief Restore previous energy values
 *  @param [in] pRes : pointer to residual values
 *  @param [in] pData : pointer to current dataset
 */
void
restoreResidual(const Emon32Cumulative_t *pRes, ECMSet_t *pData)
{
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        pData->CT[idxCT].wattHour = pRes->wattHour[idxCT];
    }
}

/*! @brief This function is called when the 1 ms timer overflows (SYSTICK).
 *         Latency is not guaranteed, so only non-timing critical things
 *         should be done here (UI update, watchdog etc)
 */
static void
evtKiloHertz()
{
    static unsigned int swStateTime;

    /* If switch is pressed >= SW_TIME_RESET, save state and reset */
    swStateTime = (SW_CLOSED == uiSWUpdate()) ? swStateTime + 1u : 0;

    if (SW_TIME_RESET <= swStateTime)
    {
        emon32SetEvent(EVT_SAVE_RESET);
    }

    uiUpdateLED(EMON_STATE_IDLE);

    /* Kick watchdog - placed in the event handler to allow reset of stuck
     * processing rather than entering the interrupt reliably
     */
    wdtKick();
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
    Emon32Config_t      e32Config;
    Emon32Cumulative_t  e32Residual;
    ECMSet_t            dataset;
    char                txBuffer[64]; /* TODO Check size of buffer */

    setup_uc();

    /* Setup DMAC for non-blocking UART (this is optional, unlike ADC) */
    uartConfigureDMA();
    uartInterruptEnable(SERCOM_UART_DBG, SERCOM_USART_INTENSET_RXC);
    uartInterruptEnable(SERCOM_UART_DBG, SERCOM_USART_INTENSET_ERROR);

    uartPutsBlocking(SERCOM_UART_DBG, "\ec== Energy Monitor 32 ==\r\n");
    loadConfiguration(&e32Config, &e32Residual);
    restoreResidual(&e32Residual, &dataset);

    emon32StateSet(EMON_STATE_ACTIVE);
    ecmInit(&e32Config);
    adcStartDMAC((uint32_t)ecmDataBuffer());

    for (;;)
    {
        /* While there is an event pending (may be set while another is
         * handled, keep looping. Enter sleep (WFI) when done.
         */
        while(0 != evtPend)
        {
            /* 1 ms timer flag */
            if (evtPend & (1u << EVT_SYSTICK_1KHz))
            {
                evtKiloHertz();
                emon32ClrEvent(EVT_SYSTICK_1KHz);
            }

            /* ADC sample set complete */
            if (evtPend & (1u << EVT_DMAC_SMP_CMPL))
            {
                ecmInjectSample();
                emon32ClrEvent(EVT_DMAC_SMP_CMPL);
            }

            /* Full cycle complete */
            if (evtPend & (1u << EVT_ECM_CYCLE_CMPL))
            {
                ecmProcessCycle();
                emon32ClrEvent(EVT_ECM_CYCLE_CMPL);
            }

            /* Report period elapsed; generate, pack, and send */
            if (evtPend & (1u << EVT_ECM_SET_CMPL))
            {
                unsigned int pktLength;
                ecmProcessSet(&dataset);
                pktLength = dataPackage(&dataset, txBuffer);
                uartPutsNonBlocking(DMA_CHAN_UART_DBG, txBuffer, pktLength);
                emon32ClrEvent(EVT_ECM_SET_CMPL);
            }

            /* Save and reset requested */
            if (evtPend & (1u << EVT_SAVE_RESET))
            {
                /* TODO Save state */
                NVIC_SystemReset();
            }

        }
        __WFI();
    };
}
