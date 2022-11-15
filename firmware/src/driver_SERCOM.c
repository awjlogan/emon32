#include "emon32_samd.h"

void
sercomSetup()
{
    /* USART is mapped to SERCOM0 (alt pins) */
    portPinDir(PIN_UART_TX, PIN_DIR_OUT);
    portPinMux(PIN_UART_TX, PORT_PMUX_PMUXE_D);
    portPinDir(PIN_UART_RX, PIN_DIR_IN);
    portPinDir(PIN_UART_RX, PORT_PMUX_PMUXE_D);

    const uint32_t baud = UART_BAUD;
    const uint64_t br = (uint64_t)65536 * (F_PERIPH - 16 * baud) / F_PERIPH;

    /* Configure clocks - runs from the 1 MHz OSC8M clock on gen 3 */
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Configure the USART */
    SERCOM0->USART.CTRLA.reg =   SERCOM_USART_CTRLA_DORD
                               | SERCOM_USART_CTRLA_MODE_USART_INT_CLK
                               | SERCOM_USART_CTRLA_RXPO(UART_PAD_RX)
                               | SERCOM_USART_CTRLA_TXPO(UART_PAD_TX);

    SERCOM0->USART.CTRLB.reg =   SERCOM_USART_CTRLB_RXEN
                               | SERCOM_USART_CTRLB_TXEN
                               | SERCOM_USART_CTRLB_CHSIZE(0);

    SERCOM0->USART.BAUD.reg = (uint16_t)br + 1u;
    SERCOM0->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;

    /* TODO If RFM69 is used, add SPI configuration on SERCOM1 */
}

void
uartPutcBlocking(char c)
{
    while (!(SERCOM0->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE));
    SERCOM0->USART.DATA.reg = c;
}

void
uartPutsBlocking(const char *s)
{
    while (*s) uartPutcBlocking(*s++);
}
