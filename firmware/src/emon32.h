#ifndef EMON32_H
#define EMON32_H

#include <stdint.h>

/* Voltage and CT setup - this is configurable, but constrained per board. For
 * example, NUM_CT can't be greater than the number of physical channels, but
 * can be less.
 */
#define NUM_V               1u
#define NUM_CT              4u
#define SAMPLE_RATE         4800u
#define SAMPLES_IN_SET      2u
#define SAMPLE_BUF_DEPTH    2u

/* Uncomment to downsample the sample rate by low pass filter
 * Otherwise, the second sample from each set will be discarded
 *
 * #define DOWNSAMPLE_DSP
 */

/* INTSRC_t contains all the event/interrupts sources. This value is shifted
 * to provide a vector of set events as bits.
 */
typedef enum {
    EVT_DMA             = 0u,
    EVT_SYSTICK_1KHz    = 1u,
    EVT_TCC             = 2u,
    EVT_UART            = 3u,
    EVT_ADC             = 4u,
    EVT_DMAC_UART_CMPL  = 5u,
    EVT_DMAC_SMP_CMPL   = 6u
} INTSRC_t;


/* SingleSampleSet_t contains a single set of V + CT ADC samples */
typedef struct __attribute__((__packed__)) SingleSampleSet {
    int16_t smpV[NUM_V];
    int16_t smpCT[NUM_CT];
} SingleSampleSet_t;

/* SampleSetPacked_t contains a set of single sample sets. This allows the DMAC
 * to blit samples across multiple sample sets, depending on processing needs
 */
typedef struct __attribute__((__packed__)) SampleSetPacked {
    SingleSampleSet_t samples[SAMPLES_IN_SET];
} SampleSetPacked_t;

/* SampleSet_t contains an unpacked set of single sample sets */
typedef struct SampleSet {
    int16_t smpV[NUM_V];
    int16_t smpCT[NUM_CT];
} SampleSet_t;

/*! @brief Set the pending event/interrupt flag for tasks that are not handled
 *        within an ISR
 *  @param [in] Event source in enum
 */
void emon32SetEvent(INTSRC_t evt);

/*! @brief Clear a pending event/interrupt flag after the task has been handled
 *  @param [in] Event source in enum
 */
void emon32ClrEvent(INTSRC_t evt);

#endif
