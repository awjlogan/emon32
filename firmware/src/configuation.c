#include "emon32_samd.h"

static unsigned int valChanged;
static Emon32Config_t *pCfg;
static char genBuf[12];

static unsigned int
getValue()
{
    char *pBuf = genBuf;
    unsigned int charCnt = 0;
    char c = 0;

    /* Buffer will fill in reverse order, done when # is received */
    while ('#' != c)
    {
        *pBuf++ = c;
        charCnt++;
        while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
        c = uartGetc(UART_SERCOM_DBG);
    }
    utilStrReverse(genBuf, charCnt);
    return utilAtoi(genBuf, ITOA_BASE10);
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

static inline void
clearTerm()
{
    uartPutsBlocking("\ec");
}

static void
menuConfiguration();
{
    char c = 0;

    while ('b' != c)
    {
        clearTerm();
        uartPutsBlocking("---- CONFIGURATION ----\n\n");

        uartPutsBlocking("0: Cycles to report: ");
        putValueEnd_10(pCfg->baseCfg.reportCycles);

        uartPutsBlocking("1: Mains frequency:  ");
        putValueEnd_10(pCfg->baseCfg.mainsFreq);


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
    /* TODO fetch SAMD unique ID */

    uartPutsBlocking("Voltage channels: ");
    putValueEnd_10(NUM_V);

    uartPutsBlocking("CT channels:      ");
    putValueEnd_10(NUM_CT);

    uartPutsBlocking("(c) Angus Logan 2022-23\n");
    uartPutsBlocking("For Bear and Moose\n\n");

    uartPutsBlocking("(b)ack");
    while ('b' != c)
    {
        while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
        c = uartGetc(UART_SERCOM_DBG);
    }
}

static void
menuBase()
{
    unsigned int validC = 0;
    char c = 'a';
    /* Clear terminal and print menu */
    clearTerm();
    uartPutsBlocking("--- emon32 ---\n\n");
    uartPutsBlocking("  0: About\n");
    uartPutsBlocking("  1: Configuration\n");
    uartPutsBlocking("  2: Voltage\n");
    uartPutsBlocking("  3: CT\n\n");

    uartPutsBlocking("Enter number, or (e)xit")
    if (valChanged)
    {
        uartPutsBlocking(" do not save, or (s)ave and exit.");
    }
    else
    {
        uartPutcBlocking('.');
    }

    while ('s' != c && 'e' != c);
    {
        while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
        c = uartGetc(UART_SERCOM_DBG);

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
            /* Terminal ping/flash */
        }
    }
    /* Warn if the changes are going to be discarded */
    if (0 != valChanged && 'e' == c)
    {
        uartPutsBlocking("Discard changes? (y/n)");
        while ('y' != c && 'n' != c)
        {
            while (0 == uartInterruptStatus(UART_SERCOM_DBG, SERCOM_USART_INTFLAG_RXC));
            c = uartGetc(UART_SERCOM_DBG);
        }
        c = ('y' == c) ? 'e' : 's';
    }

    if ('s' == c)
    {
        /* save configuration */
    }

    /* Soft reset the core, any saved values will be loaded at boot */
    SCB->AIRCR |= SCB_AIRCR_SYSRESETREQ_Msk;
}

void
configEnter(Emon32Config_t *pConfig)
{
    pCfg = pConfig;
    uartInterruptEnable(UART_SERCOM_DBG, SERCOM_USART_INTENSET_RXC);
    menuBase();
}
