#ifdef HOSTED
    #include <string.h>
    #include <stdio.h>
    #include "configuration.h"
    #include "util.h"

    #define SERCOM_UART_DBG 0u

    /* Override UART outputs */
    void
    uartPutsBlocking(uint32_t Sercom, const char *pSrc)
    {
        (void)Sercom;
        printf("%s", pSrc);
    }

    void
    uartPutcBlocking(uint32_t Sercom, char c)
    {
        (void)Sercom;
        printf("%c", c);
    }
#else
    #include "emon32_samd.h"
#endif

#define GENBUF_W        16u

static unsigned int     valChanged;
static Emon32Config_t   *pCfg;
static char             genBuf[GENBUF_W];

/*! @brief Fetch the board's unique ID, and place in genBuf
 *         For SAMD series, there is a unique value in the IC
 */
static inline uint32_t
getUniqueID(unsigned int idx)
{
    #ifdef HOSTED
        return 1 << (idx * 2);
    #else
        /* Section 9.6 Serial Number */
        const uint32_t id_addr_lut[4] = {
            0x0080A00C, 0x0080A40, 0x0080A044, 0x0080A048
        };
        return *(volatile uint32_t *)id_addr_lut[idx];
    #endif
}

static void
getInputStr()
{
    char *pBuf = genBuf;
    unsigned int charCnt = 0;
    char c = 0;

    /* Exit when # is received, or out of bounds */
    while ('#' != c && (charCnt < GENBUF_W))
    {
        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif
        if ('#' != c)
        {
            *pBuf++ = c;
            charCnt++;
        }
    }
}

static unsigned int
getValue()
{
    getInputStr();
    return utilAtoi(genBuf, ITOA_BASE10);
}

static float
getValue_float()
{
    getInputStr();
    return utilAtof(genBuf);
}

static void
infoEdit()
{
    uartPutsBlocking(SERCOM_UART_DBG, "\r\nTo edit, enter the index, the value (base 10), then #. Go (b)ack\r\n");
}

static void
putValueEnd()
{
    uartPutsBlocking(SERCOM_UART_DBG, genBuf);
    uartPutcBlocking(SERCOM_UART_DBG, '\r');
    uartPutcBlocking(SERCOM_UART_DBG, '\n');
}

static void
putValueEnd_10(unsigned int val)
{
    (void)utilItoa(genBuf, val, ITOA_BASE10);
    putValueEnd();
}

static void
putValueEnd_Float(float val)
{
    (void)utilFtoa(genBuf, val);
    putValueEnd();
}

static inline void
clearTerm()
{
    uartPutsBlocking(SERCOM_UART_DBG, "\ec");
}

static void
menuVoltageChan(unsigned int chanV)
{
    char c = 0;
    unsigned int idxChange = 0;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking(SERCOM_UART_DBG, "---- VOLTAGE CHANNEL ");
        (void)utilItoa(genBuf, chanV, ITOA_BASE10);
        uartPutsBlocking(SERCOM_UART_DBG, genBuf);
        uartPutsBlocking(SERCOM_UART_DBG, " ----\r\n\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "0: Conversion factor: ");
        putValueEnd_Float(pCfg->voltageCfg[chanV].voltageCal);
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        if ('0' == c)
        {
            float val;
            idxChange = c - '0';
            val = getValue_float();

            switch (idxChange)
            {
                case 0:
                    pCfg->voltageCfg[chanV].voltageCal = val;
                    valChanged = 1u;
                    break;
                default:
                    break;
            }
        }
    }
}

static void
menuVoltage()
{
    char c = 0;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking(SERCOM_UART_DBG, "---- VOLTAGE CHANNELS ----\r\n\r\n");
        for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
        {
            (void)utilItoa(genBuf, idxV, ITOA_BASE10);
            uartPutsBlocking(SERCOM_UART_DBG, genBuf);
            uartPutsBlocking(SERCOM_UART_DBG, ": Voltage Channel ");
            uartPutsBlocking(SERCOM_UART_DBG, genBuf);
            uartPutsBlocking(SERCOM_UART_DBG, "\r\n");
        }
        uartPutsBlocking(SERCOM_UART_DBG, "Enter the channel index to enter configuration\r\n\r\n");

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        if ('b' != c)
        {
            menuVoltageChan(c - '0');
        }
    }
}

static void
menuCTChan(unsigned int chanCT)
{
    char c = 0;
    unsigned int idxChange = 0;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking(SERCOM_UART_DBG, "---- CT CHANNEL ");
        (void)utilItoa(genBuf, chanCT, ITOA_BASE10);
        uartPutsBlocking(SERCOM_UART_DBG, genBuf);
        uartPutsBlocking(SERCOM_UART_DBG, " ----\r\n\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "0: Conversion factor:   ");
        putValueEnd_Float(pCfg->ctCfg[chanCT].ctCal);
        uartPutsBlocking(SERCOM_UART_DBG, "1: Phase calibration X: ");
        putValueEnd_10(pCfg->ctCfg[chanCT].phaseX);
        uartPutsBlocking(SERCOM_UART_DBG, "1: Phase calibration Y: ");
        putValueEnd_10(pCfg->ctCfg[chanCT].phaseY);
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        if ('0' == c)
        {
            float   val_f;
            int16_t val_fixed;
            idxChange = c - '0';

            switch (idxChange)
            {
                case 0:
                    val_f = getValue_float();
                    pCfg->ctCfg[chanCT].ctCal = val_f;
                    valChanged = 1u;
                    break;
                case 1:
                    val_fixed = getValue();
                    pCfg->ctCfg[chanCT].phaseX = val_fixed;
                    valChanged = 1u;
                    break;
                case 2:
                    val_fixed = getValue();
                    pCfg->ctCfg[chanCT].phaseY = val_fixed;
                    valChanged = 1u;
                    break;
                default:
                    break;
            }
        }
    }
}

static void
menuCT()
{
    char c = 0;
    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking(SERCOM_UART_DBG, "---- CT CHANNELS ----\r\n\r\n");
        for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
        {
            (void)utilItoa(genBuf, idxCT, ITOA_BASE10);
            uartPutsBlocking(SERCOM_UART_DBG, genBuf);
            uartPutsBlocking(SERCOM_UART_DBG, ": CT Channel ");
            uartPutsBlocking(SERCOM_UART_DBG, genBuf);
            uartPutsBlocking(SERCOM_UART_DBG, "\r\n");
        }
        uartPutsBlocking(SERCOM_UART_DBG, "Enter the channel index to enter configuration\r\n\r\n");

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        if ('b' != c)
        {
            menuCTChan(c - '0');
        }
    }
}

static void
menuConfiguration()
{
    char c = 0;
    unsigned int idxChange;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking(SERCOM_UART_DBG, "---- CONFIGURATION ----\r\n\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "0: Node ID:                  ");
        putValueEnd_10(pCfg->baseCfg.nodeID);
        uartPutsBlocking(SERCOM_UART_DBG, "1: Cycles to report:         ");
        putValueEnd_10(pCfg->baseCfg.reportCycles);
        uartPutsBlocking(SERCOM_UART_DBG, "2: Mains frequency (Hz):     ");
        putValueEnd_10(pCfg->baseCfg.mainsFreq);
        uartPutsBlocking(SERCOM_UART_DBG, "3: Discard initial cycles:   ");
        putValueEnd_10(pCfg->baseCfg.equilCycles);
        uartPutsBlocking(SERCOM_UART_DBG, "4: Zero crossing hysteresis: ");
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        if ((c >= '0') && (c <= '4'))
        {
            uint32_t val;
            idxChange = c - '0';
            val = getValue();

            switch (idxChange)
            {
                case 0:
                    pCfg->baseCfg.nodeID = val;
                    valChanged = 1u;
                    break;
                case 1:
                    pCfg->baseCfg.reportCycles = val;
                    valChanged = 1u;
                    break;
                case 2:
                    pCfg->baseCfg.mainsFreq = val;
                    valChanged = 1u;
                    break;
                case 3:
                    pCfg->baseCfg.equilCycles = val;
                    valChanged = 1u;
                    break;
                case 4:
                    pCfg->baseCfg.zcHyst = val;
                    valChanged = 1u;
                    break;
                default:
                    break;
            }
        }
    }
}

static void
menuAbout()
{
    char c = 0;

    clearTerm();
    uartPutsBlocking(SERCOM_UART_DBG, "---- ABOUT ----\r\n\r\n");
    uartPutsBlocking(SERCOM_UART_DBG, "Firmware version: ");

    (void)utilItoa(genBuf, VERSION_FW_MAJ, ITOA_BASE10);
    uartPutsBlocking(SERCOM_UART_DBG, genBuf);
    uartPutcBlocking(SERCOM_UART_DBG, '.');
    putValueEnd_10(VERSION_FW_MIN);

    uartPutsBlocking(SERCOM_UART_DBG, "Serial number:    ");
    for (unsigned int i = 0; i < 4; i++)
    {
        utilItoa(genBuf, getUniqueID(i), ITOA_BASE16);
        uartPutsBlocking(SERCOM_UART_DBG, genBuf);
    }
    uartPutsBlocking(SERCOM_UART_DBG, "\r\n\r\n");

    uartPutsBlocking(SERCOM_UART_DBG, "Voltage channels: ");
    putValueEnd_10(NUM_V);

    uartPutsBlocking(SERCOM_UART_DBG, "CT channels:      ");
    putValueEnd_10(NUM_CT);

    uartPutsBlocking(SERCOM_UART_DBG, "\r\n(c) Angus Logan 2022-23\r\n");
    uartPutsBlocking(SERCOM_UART_DBG, "For Bear and Moose\r\n\r\n");
    uartPutsBlocking(SERCOM_UART_DBG, "(b)ack");
    while ('b' != c)
    {
        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif
    }
}

static void
menuBase()
{
    char c = 'a';

    while ('s' != c && 'e' != c)
    {
        /* Clear terminal and print menu */
        clearTerm();

        uartPutsBlocking(SERCOM_UART_DBG, "--- emon32 ---\r\n\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "  0: About\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "  1: Configuration\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "  2: Voltage\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "  3: CT\r\n\r\n");
        uartPutsBlocking(SERCOM_UART_DBG, "Enter number, or (e)xit");

        if (valChanged)
        {
            uartPutsBlocking(SERCOM_UART_DBG, " do not save, or (s)ave and exit.");
        }
        else
        {
            uartPutcBlocking(SERCOM_UART_DBG, '.');
        }

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(SERCOM_UART_DBG);
        #endif

        switch (c)
        {
            case '0':
                menuAbout();
                break;
            case '1':
                menuConfiguration();
                break;
            case '2':
                menuVoltage();
                break;
            case '3':
                menuCT();
                break;
            /* Fall through save or exit */
            case 's':
            case 'e':
                break;
            default:
                uartPutcBlocking(SERCOM_UART_DBG, '\a');
                /* Terminal ping/flash */
        }
    }

    /* Warn if the changes are going to be discarded */
    if (0 != valChanged && 'e' == c)
    {
        uartPutsBlocking(SERCOM_UART_DBG, "Discard changes? (y/n)");
        while ('y' != c && 'n' != c)
        {
            #ifdef HOSTED
                c = getchar();
                if ('\n' == c)
                    c = getchar();
            #else
                while (0 == (uartInterruptStatus(SERCOM_UART_DBG) & SERCOM_USART_INTFLAG_RXC));
                c = uartGetc(SERCOM_UART_DBG);
            #endif

        }
        c = ('y' == c) ? 'e' : 's';
    }

    if ('s' == c)
    {
        #ifndef HOSTED
            /* Save configuration. The first byte of the EEPROM is written as 0
             * to mark that the stored data are valid.
             */
            c = 0;
            eepromWrite(EEPROM_BASE_ADDR, (void *)&c, 1u);
            eepromWrite(EEPROM_BASE_ADDR + EEPROM_PAGE_SIZE, pCfg, sizeof(Emon32Config_t));
        #endif
        uartPutsBlocking(SERCOM_UART_DBG, "Configuration saved.\r\n");
    }
}

void
configEnter(Emon32Config_t *pConfig)
{
    pCfg = pConfig;
    menuBase();
}
