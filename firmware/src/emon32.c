#include "emon32_samd.h"

static inline void
setup_samd()
{
    clk_setup();
    timer_setup();
};

int
main()
{
    setup_samd();
    while(1);
}
