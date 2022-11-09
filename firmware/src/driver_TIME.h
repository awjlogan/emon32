#ifndef DRIVER_TIME_H
#define DRIVER_TIME_H

#include <stdint.h>

/* @brief       Sets up the timer unit */
void timer_setup();

/* @brief       Blocking delay. Use with caution.
 * @param [in]  Delay period in ms, maximum is (2^16)-1 ms
 */
void timer_delay_ms(uint16_t delay);

/* @brief       Blocking delay. Use with caution.
 * @param [in]  Delay period is us, maximum is (2^32)-1 us
 * TODO Check microsecond resolution
 */
void timer_delay_us(uint32_t delay);

#endif
