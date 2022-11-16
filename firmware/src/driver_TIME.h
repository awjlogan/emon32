#ifndef DRIVER_TIME_H
#define DRIVER_TIME_H

#include <stdint.h>

/* @brief       Sets up the timer unit */
void timerSetup();

/* @brief       Blocking delay. Use with caution.
 * @param [in]  Delay period in ms, maximum is (2^16)-1 ms
 */
void timerDelay_ms(uint16_t delay);

/* @brief       Blocking delay. Use with caution.
 * @param [in]  Delay period in us
 */
void timerDelay_us(uint32_t delay);

#endif
