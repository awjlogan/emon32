#include "emon32_samd.h"

SwitchState_t
uiUpdateSW()
{
    static uint32_t sw_in;
    static SwitchState_t sw_last = SW_OPEN;

    /* On each 1 kHz tick, shift the current button state in. When all
     * bits are set or cleared, the button is in a stable state.
     */
    sw_in = sw_in << 1u;
    if (0 != portPinValue(PIN_SW)) sw_in++;
    if (0 == sw_in)
    {
        /* Switch is closed */
        if (SW_OPEN == sw_last)
        {
            sw_last = SW_PRESSED;
        }
        else if (SW_PRESSED == sw_last)
        {
            sw_last = SW_CLOSED;
        }
    }
    else
    {
        /* Switch is OPEN */
        if (SW_CLOSED == sw_last)
        {
            sw_last = SW_RELEASED;
        }
        else if (SW_RELEASED == sw_last)
        {
            sw_last = SW_OPEN;
        }
    }
    return sw_last;
}

void
uiUpdateLED(EmonState_t estate)
{
    static unsigned int ticks;
    if (EMON_IDLE == estate)
    {
        /* TODO LED to pulse */
        portPinDrv(PIN_LED, PIN_DRV_CLR);
    }
    else if (EMON_ACTIVE == estate)
    {
        portPinDrv(PIN_LED, PIN_DRV_SET);
    }
    else
    {
        /* Flash at 1 Hz */
        ticks++;
        if (500u == ticks)
        {
            ticks = 0u;
            portPinDrv(PIN_LED, PIN_DRV_TGL);
        }
    }
}
