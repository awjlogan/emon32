#ifndef EMON32_H
#define EMON32_H

#include <stdint.h>

/* INTSRC_t contains all the event/interrupts sources. */
typedef enum {
    EVT_DMA             = 0u,
    EVT_SYSTICK_1KHz    = 1u,
    EVT_TCC             = 2u,
    EVT_UART            = 3u,
    EVT_ADC             = 4u
} INTSRC_t;

/* @brief Set the pending event/interrupt flag for tasks that are not handled
 *        within an ISR
 * @param [in] Event source in enum
 */
void emon32SetEvent(INTSRC_t evt);

/* @brief Clear a pending event/interrupt flag after the task has been handled
 * @param [in] Event source in enum
 */
void emon32ClrEvent(INTSRC_t evt);

#endif
