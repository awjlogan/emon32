#include "emon32_samd.h"

void
evsysSetup()
{
    /* Enable APB clock, connect to generator 3 (OSC8M @ F_PERIPH)
     * Each EVSYS channel has a separate GCLK channel
     */
    PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;
    GCLK->CLKCTRL.reg =   GCLK_CLKCTRL_ID(EVSYS_GCLK_ID_0)
                        | GCLK_CLKCTRL_GEN(3u)
                        | GCLK_CLKCTRL_CLKEN;

    /* Connect TC1 -> ADC (Section 23.6.2.1) - select channel N-1 */
    EVSYS->USER.reg =   EVSYS_USER_USER(EVSYS_ID_USER_ADC_START)
                      | EVSYS_USER_CHANNEL(1u);
    EVSYS->CHANNEL.reg =   EVSYS_CHANNEL_CHANNEL(1u)
                         | EVSYS_CHANNEL_EDGSEL(EVSYS_CHANNEL_EDGSEL_RISING_EDGE)
                         | EVSYS_CHANNEL_PATH_RESYNCHRONIZED
                         | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TC1_OVF);
}
