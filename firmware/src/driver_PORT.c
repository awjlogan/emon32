#include "emon32_samd.h"

void
portSetup()
{
    /* GPIOs */
    portPinDir(PIN_EXTINT, PIN_DIR_OUT);
    portPinDir(PIN_GEN_STATUS, PIN_DIR_OUT);
    portPinDir(PIN_LED, PIN_DIR_OUT);
    portPinDir(PIN_SW, PIN_DIR_IN);

    /* Pull up on switch pin */
    portPinCfg(PIN_SW, PORT_PINCFG_PULLEN, PIN_CFG_SET);
    portPinDrv(PIN_SW, PIN_DRV_SET);

    /* Unused pins: input, pull down (Table 2201) */
    const uint8_t pinsUnused[3] = {22u, 34u, 25u};
    for (unsigned int idxPin = 0; idxPin < 3u; idxPin++) {
        const uint8_t pin = pinsUnused[idxPin];
        portPinDir(pin, PIN_DIR_IN);
        portPinCfg(pin, PORT_PINCFG_PULLEN, PIN_CFG_SET);
    }
}

void
portPinDir(unsigned int pin, PINDIR_t mode)
{
    if (PIN_DIR_IN == mode)
    {
        PORT->Group[0].DIRCLR.reg = (1u << pin);
    }
    else
    {
        PORT->Group[0].DIRSET.reg = (1u << pin);
    }
    PORT->Group[0].PINCFG[pin].reg |= PORT_PINCFG_INEN;
}

void
portPinMux(unsigned int pin, unsigned int mux)
{
    PORT->Group[0].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN;
    if (pin & 1u)
    {
        PORT->Group[0].PMUX[pin >> 1].bit.PMUXO = mux;
    }
    else
    {
        PORT->Group[0].PMUX[pin >> 1].bit.PMUXE = mux;
    }
}

void
portPinCfg(unsigned int pin, unsigned int cfg, PINCFG_t cs)
{
    if (PIN_CFG_SET == cs)
    {
        PORT->Group[0].PINCFG[pin].reg |= cfg;
    }
    else
    {
        PORT->Group[0].PINCFG[pin].reg &= ~cfg;
    }
}

void
portPinDrv(unsigned int pin, PINDRV_t drv)
{
    if (PIN_DRV_CLR == drv)
    {
        PORT->Group[0].OUTCLR.reg = (1u << pin);
    }
    else if (PIN_DRV_SET == drv)
    {
        PORT->Group[0].OUTSET.reg = (1u << pin);
    }
    else
    {
        PORT->Group[0].OUTTGL.reg = (1u << pin);
    }
}

unsigned int
portPinValue(unsigned int pin)
{
    unsigned int ret;
    ret = (0u == (PORT->Group[0].IN.reg & (1u << pin))) ? 0u : 1u;
    return ret;
}
