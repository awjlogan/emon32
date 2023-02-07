#include <stddef.h>

#include "board_def.h"
#include "emon32.h"
#include "eeprom.h"
#include "emon32_samd.h"
#include "emon_CM.h"

#define EEPROM_WL_NUM_BLK   EEPROM_WL_SIZE / EEPROM_WL_SIZE_BLK

/* Persistent state variables */
static volatile uint32_t    evtPend;
static volatile EmonState_t emonState = EMON_STATE_IDLE;

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

/*! @brief The default configuration state of the system */
static inline void
defaultConfiguration(Emon32Config_t *pCfg, Emon32Cumulative_t *pRes)
{
    /* Default configuration: single phase, 50 Hz, 240 VAC */
    pCfg->baseCfg.mainsFreq = 50u;      /* Mains frequency */
    pCfg->baseCfg.reportCycles = 500u;  /* 10 s @ 50 Hz */

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
        pRes->report.wattHour[idxCT] = 0;
    }
    pRes->report.pulseCnt = 0;
}

/*! @brief This function handles loading of configuration data
 *  @param [in] pCfg : pointer to the configuration struct
 */
static inline void
loadConfiguration(Emon32Config_t *pCfg)
{
    unsigned int systickCnt = 0u;
    unsigned int seconds = 3u;

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
            wdtFeed();
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
}

/*! @brief Total energy across all CTs
 *  @param [in] pData : pointer to data setup
 *  @return : sum of Wh for all CTs
 */
uint32_t
totalEnergy(const ECMSet_t *pData)
{
    uint32_t totalEnergy = 0;
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        totalEnergy += pData->CT[idxCT].wattHour;
    }
    return totalEnergy;
}

/*! @brief Load cumulative energy and pulse values
 *  @param [in] pEEPROM : pointer to EEPROM configuration
 *  @param [in] pData : pointer to current dataset
 */
static void
loadCumulative(eepromPktWL_t *pPkt, ECMSet_t *pData)
{
    Emon32Cumulative_t data;
    pPkt->pData = &data;
    eepromReadWL(pPkt);

    /* Store into current result set */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        pData->CT[idxCT].wattHour = data.report.wattHour[idxCT];
    }

    pData->pulseCnt = data.report.pulseCnt;
}

/*! @brief Store cumulative energy and pulse values
 *  @param [in] pRes : pointer to cumulative values
 */
static void
storeCumulative(eepromPktWL_t *pPkt, const ECMSet_t *pData)
{
    Emon32Cumulative_t data;
    pPkt->pData = &data;

    /* Copy data and calculate CRC */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        data.report.wattHour[idxCT] = pData->CT[idxCT].wattHour;
    }
    data.report.pulseCnt = pData->pulseCnt;

    data.crc = crc16_ccitt(&data.report, sizeof(Emon32Report_t));

    eepromWriteWL(pPkt);
}

/*! @brief This function is called when the 1 ms timer overflows (SYSTICK).
 *         Latency is not guaranteed, so only non-timing critical things
 *         should be done here (UI update, watchdog etc)
 */
static void
evtKiloHertz()
{
    /* Kick watchdog - placed in the event handler to allow reset of stuck
     * processing rather than entering the interrupt reliably
     */
    wdtFeed();
}


/*! @brief Check if an event source is active
 *  @param [in] : event source to check
 *  @return : 1 if pending, 0 otherwise
 */
static inline unsigned int
evtPending(INTSRC_t evt)
{
    if (evtPend & (1u << evt))
    {
        return 1u;
    }
    else
    {
        return 0;
    }
}


/*! @brief Setup the microcontoller. This function must be called first. An
 *         implementation must provide all the functions that are called.
 *         These can be empty if they are not used.
 */
static inline void
setup_uc()
{
    clkSetup();
    timerSetup();
    portSetup();
    dmacSetup();
    sercomSetup();
    adcSetup();
    evsysSetup();
    eicSetup();
    wdtSetup(WDT_PER_4K);
};

int
main()
{
    Emon32Config_t      e32Config;
    ECMSet_t            dataset;
    eepromPktWL_t       eepromPkt;
    uint32_t            lastStoredWh;
    uint32_t            latestWh;
    char                txBuffer[64]; /* TODO Check size of buffer */
    uint32_t            pulseTimeSince = 0;

    setup_uc();

    /* Setup DMAC for non-blocking UART (this is optional, unlike ADC) */
    uartConfigureDMA();
    uartInterruptEnable(SERCOM_UART_DBG, SERCOM_USART_INTENSET_RXC);
    uartInterruptEnable(SERCOM_UART_DBG, SERCOM_USART_INTENSET_ERROR);

    uartPutsBlocking(SERCOM_UART_DBG, "\ec== Energy Monitor 32 ==\r\n");

    /* Load stored values from non-volatile memory */
    eepromPkt.addr_base = EEPROM_WL_OFFSET;
    eepromPkt.blkCnt = EEPROM_WL_NUM_BLK;
    eepromPkt.dataSize = sizeof(Emon32Cumulative_t);
    eepromPkt.idxNextWrite = -1;

    loadConfiguration(&e32Config);
    loadCumulative(&eepromPkt, &dataset);
    lastStoredWh = totalEnergy(&dataset);

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
            if (evtPending(EVT_SYSTICK_1KHz))
            {
                evtKiloHertz();
                if (0 != pulseTimeSince)
                {
                    pulseTimeSince--;
                }
                emon32ClrEvent(EVT_SYSTICK_1KHz);
            }

            /* Full cycle complete */
            if (evtPending(EVT_ECM_CYCLE_CMPL))
            {
                ecmProcessCycle();
                emon32ClrEvent(EVT_ECM_CYCLE_CMPL);
            }

            /* Pulse count interrupt */
            if (evtPending(EVT_EIC_PULSE))
            {
                if (0 == pulseTimeSince)
                {
                    dataset.pulseCnt++;
                }
            }

            /* Report period elapsed; generate, pack, and send */
            if (evtPending(EVT_ECM_SET_CMPL))
            {
                unsigned int pktLength;
                unsigned int energyOverflow;

                ecmProcessSet(&dataset);
                pktLength = dataPackage(&dataset, txBuffer);
                uartPutsNonBlocking(DMA_CHAN_UART_DBG, txBuffer, pktLength);

                #ifdef TRANSMIT_ESP8266
                uartPutsNonBlocking(DMA_CHAN_UART_DATA, txBuffer, pktLength);
                #endif

                /* Store cumulative values if over threshold */
                latestWh = totalEnergy(&dataset);

                /* Catch overflow of energy. This corresponds to 4 MWh(!), so
                 * unlikely to happen, but handle safely.
                 */
                energyOverflow = (latestWh < lastStoredWh) ? 1u : 0;
                if (0 != energyOverflow)
                {
                    uartPutsBlocking(SERCOM_UART_DBG, "\r\n> Cumulative energy overflowed counter!");
                }

                if (((latestWh - lastStoredWh) > DELTA_WH_STORE) || energyOverflow)
                {
                    storeCumulative(&eepromPkt, &dataset);
                    lastStoredWh = latestWh;
                }
                emon32ClrEvent(EVT_ECM_SET_CMPL);
            }

            /* When the EEPROM page buffer has been filled, start the write
             * timer to ensure that the write completes successfully. Retry if
             * the timer is in use.
             */
            if (evtPending(EVT_DMAC_I2C_CMPL))
            {
                if (-1 != timerDelayNB_us(EEPROM_WR_TIME))
                {
                    emon32ClrEvent(EVT_DMAC_I2C_CMPL);
                }
            }

            /* Start writing the next EEPROM block. If complete, disconnect
             * timer from the NVIC.
             */
            if (evtPending(EVT_TIMER_MC))
            {
                if (EEPROM_WR_COMPLETE == eepromWrite(0, NULL, 0))
                {
                    NVIC_DisableIRQ(TC2_IRQn);
                    timerDisable();
                }
                emon32ClrEvent(EVT_TIMER_MC);
            }

            /* Save and reset requested */
            if (evtPending(EVT_SAVE_RESET))
            {
                storeCumulative(&eepromPkt, &dataset);
                NVIC_SystemReset();
            }
        }
        /* Sleep if nothing pending */
        __WFI();
    };
}
