#ifdef HOSTED
    #include <string.h>
    #include <stdio.h>
    #include "configuration.h"
    #include "util.h"

    /* Override UART outputs */
    void
    uartPutsBlocking(const char *pSrc)
    {
        printf("%s", pSrc);
    }

    void
    uartPutcBlocking(char c)
    {
        printf("%c", c);
    }
#else
    #include "emon32_samd.h"
#endif

static unsigned int valChanged;
static Emon32Config_t *pCfg;
static char genBuf[12];

/*! @brief Fetch the board's unique ID, and place in genBuf
 *         For SAMD series, there is a unique value in the IC
 */
static void
getUniqueID(unsigned int idx)
{
    uint32_t val;
    #ifdef HOSTED
        val = 1 << (idx * 2);
    #else
        /* Section 9.6 Serial Nuber */
        switch (idx)
        {
            case 0:
                val = *(volatile uint32_t *)(0x0080A00C);
                break;
            case 1:
                val = *(volatile uint32_t *)(0x0080A040);
                break;
            case 2:
                val = *(volatile uint32_t *)(0x0080A040);
                break;
            case 3:
                val = *(volatile uint32_t *)(0x0080A040);
                break;
            default:
                uartPutsBlocking("Panic!\n");
        }
    #endif
    utilItoa(genBuf, val, ITOA_BASE16);
}

static unsigned int
getValue()
{
    char *pBuf = genBuf;
    unsigned int charCnt = 0;
    char c = 0;

    /* Buffer will fill in reverse order, done when # is received */
    while ('#' != c)
    {
        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
        #endif
        if ('#' != c && '\n' != c)
        {
            *pBuf++ = c;
            printf("int: %c\n", c);
            charCnt++;
        }
    }
    utilStrReverse(genBuf, charCnt);
    return utilAtoi(genBuf, ITOA_BASE10);
}

static float
getValue_float()
{
    char *pBuf = genBuf;
    unsigned int charCnt = 0;
    char c = 0;

    /* Buffer will fill in reverse order, done when # is received */
    while ('#' != c)
    {
        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
        #endif
        if ('#' != c && '\n' != c)
        {
            *pBuf++ = c;
            printf("int: %c\n", c);
            charCnt++;
        }
    }
    utilStrReverse(genBuf, charCnt);
    return utilAtof(genBuf);
}

static void
infoEdit()
{
    uartPutsBlocking("\nTo edit, enter the index, the value (base 10), then #. Go (b)ack\n");
}

static void
putValueEnd_10(unsigned int val)
{
    (void)utilItoa(genBuf, val, ITOA_BASE10);
    uartPutsBlocking(genBuf);
    uartPutcBlocking('\n');
}

static void
putValueEnd_Float(float val)
{
    (void)utilFtoa(genBuf, val);
    uartPutsBlocking(genBuf);
    uartPutcBlocking('\n');
}

static inline void
clearTerm()
{
    uartPutsBlocking("\ec");
}

static void
menuVoltageChan(unsigned int chanV)
{
    char c = 0;
    unsigned int idxChange = 0;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking("---- VOLTAGE CHANNEL ");
        (void)utilItoa(genBuf, chanV, ITOA_BASE10);
        uartPutsBlocking(genBuf);
        uartPutsBlocking(" ----\n\n");
        uartPutsBlocking("0: Conversion factor: ");
        putValueEnd_Float(pCfg->voltageCfg[chanV].voltageCal);
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
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
        uartPutsBlocking("---- VOLTAGE CHANNELS ----\n\n");
        for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
        {
            (void)utilItoa(genBuf, idxV, ITOA_BASE10);
            uartPutsBlocking(genBuf);
            uartPutsBlocking(": Voltage Channel ");
            uartPutsBlocking(genBuf);
            uartPutsBlocking("\n");
        }
        uartPutsBlocking("Enter the channel index to enter configuration\n\n");

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
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
        uartPutsBlocking("---- CT CHANNEL ");
        (void)utilItoa(genBuf, chanCT, ITOA_BASE10);
        uartPutsBlocking(genBuf);
        uartPutsBlocking(" ----\n\n");
        uartPutsBlocking("0: Conversion factor:   ");
        putValueEnd_Float(pCfg->ctCfg[chanCT].ctCal);
        uartPutsBlocking("1: Phase calibration X: ");
        putValueEnd_10(pCfg->ctCfg[chanCT].phaseX);
        uartPutsBlocking("1: Phase calibration Y: ");
        putValueEnd_10(pCfg->ctCfg[chanCT].phaseY);
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
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
        uartPutsBlocking("---- CT CHANNELS ----\n\n");
        for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
        {
            (void)utilItoa(genBuf, idxCT, ITOA_BASE10);
            uartPutsBlocking(genBuf);
            uartPutsBlocking(": CT Channel ");
            uartPutsBlocking(genBuf);
            uartPutsBlocking("\n");
        }
        uartPutsBlocking("Enter the channel index to enter configuration\n\n");

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
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
        uartPutsBlocking("---- CONFIGURATION ----\n\n");
        uartPutsBlocking("0: Cycles to report:        ");
        putValueEnd_10(pCfg->baseCfg.reportCycles);
        uartPutsBlocking("1: Mains frequency (Hz):    ");
        putValueEnd_10(pCfg->baseCfg.mainsFreq);
        uartPutsBlocking("2: Discard initial samples: ");
        putValueEnd_10(pCfg->baseCfg.equilCycles);
        infoEdit();

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
        #endif

        if ('0' == c || '1' == c || '2' == c)
        {
            uint32_t val;
            idxChange = c - '0';
            val = getValue();

            switch (idxChange)
            {
                case 0:
                    pCfg->baseCfg.reportCycles = val;
                    valChanged = 1u;
                    break;
                case 1:
                    pCfg->baseCfg.mainsFreq = val;
                    valChanged = 1u;
                    break;
                case 2:
                    pCfg->baseCfg.equilCycles = val;
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
    uartPutsBlocking("---- ABOUT ----\n\n");
    uartPutsBlocking("Firmware version: ");

    (void)utilItoa(genBuf, VERSION_FW_MAJ, ITOA_BASE10);
    uartPutsBlocking(genBuf);
    uartPutcBlocking('.');
    putValueEnd_10(VERSION_FW_MIN);

    uartPutsBlocking("Serial number:    ");
    for (unsigned int i = 0; i < 4; i++)
    {
        getUniqueID(i);
        uartPutsBlocking(genBuf);
    }
    uartPutsBlocking("\n\n");

    uartPutsBlocking("Voltage channels: ");
    putValueEnd_10(NUM_V);

    uartPutsBlocking("CT channels:      ");
    putValueEnd_10(NUM_CT);

    uartPutsBlocking("\n(c) Angus Logan 2022-23\n");
    uartPutsBlocking("For Bear and Moose\n\n");
    uartPutsBlocking("(b)ack");
    while ('b' != c)
    {
        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
        #endif
    }
}

static void
menuBase()
{
    unsigned int validC = 0;
    char c = 'a';

    while ('s' != c && 'e' != c)
    {
        /* Clear terminal and print menu */
        clearTerm();

        uartPutsBlocking("--- emon32 ---\n\n");
        uartPutsBlocking("  0: About\n");
        uartPutsBlocking("  1: Configuration\n");
        uartPutsBlocking("  2: Voltage\n");
        uartPutsBlocking("  3: CT\n\n");
        uartPutsBlocking("Enter number, or (e)xit");

        if (valChanged)
        {
            uartPutsBlocking(" do not save, or (s)ave and exit.");
        }
        else
        {
            uartPutcBlocking('.');
        }

        #ifdef HOSTED
            c = getchar();
            if ('\n' == c)
                c = getchar();
        #else
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
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
                uartPutcBlocking('\a');
                /* Terminal ping/flash */
        }
    }
    /* Warn if the changes are going to be discarded */
    if (0 != valChanged && 'e' == c)
    {
        uartPutsBlocking("Discard changes? (y/n)");
        while ('y' != c && 'n' != c)
        {
            #ifdef HOSTED
                c = getchar();
                if ('\n' == c)
                    c = getchar();
            #else
                while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
                c = uartGetc(UART_SERCOM_DBG);
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
        uartPutsBlocking("Configuration saved.\n");
    }
}

void
configEnter(Emon32Config_t *pConfig)
{
    pCfg = pConfig;
    menuBase();
}
