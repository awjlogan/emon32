#include "emon32_samd.h"

static inline void
setup_uc()
{
    clkSetup();
    timerSetup();
    portSetup();
    sercomSetup();
};

int
main()
{
    setup_uc();

    uartPutcBlocking('a');
    while(1) uartPutsBlocking("\r\n== Energy Monitor 32 ==\r\n");
}
