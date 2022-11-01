#ifndef EMONLIBCM_CORE_H
#define EMONLIBCM_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Device specific defines
 *****************************************************************************/

#define NUM_CHAN_V          1u
#define NUM_CHAN_CT         7u

/******************************************************************************
 * Type definitions
 *****************************************************************************/

typedef enum {
    INIT_SUCCESS,           // Init was successful
    INIT_FAIL_ENABLED,      // Init failed as currently enabled
    ENABLE_SUCCESS,
    ENABLE_FAIL_ENABLED,
} ELC_STATUS_t;

struct ADC_samples {
    /* Voltage samples */
    int16_t     adcV[NUM_CHAN_V];
    /* CT samples */
    int16_t     adcCT[NUM_CHAN_CT];
};

struct VAccumulator {
    uint32_t    sumV_sqr;
    int32_t     sumV_deltas;
};

struct CTAccumulator {
    uint32_t    sumPA;
    uint32_t    sumPB;
    uint32_t    sumI_sqr;
    int32_t     sumI_deltas;
};

struct Accumulator {
    struct VAccumulator     processV[NUM_CHAN_V];
    struct CTAccumulator    processI[NUM_CHAN_CT];
    uint8_t                 num_samples;
};

struct ELC_result_CT_float {
    float rmsCT;
    float powerFactor;
    float realPower;
    float wattHour;
};

struct ELC_result_float {
    float                       rmsV[NUM_CHAN_V];
    struct ELC_result_CT_float  resultCT[NUM_CHAN_CT];
};

struct ELC_CONFIG {
    /* Number of mains cycles before reporting */
    uint32_t            num_report_cycles;
    /* Depth of sample buffer; must be power of 2 */
    uint8_t             depth_ADC_raw;
    /* Pointer to circular array of raw (signed) ADC values */
    struct ADC_samples  *pADC_raw;
};

/******************************************************************************
 * Function prototypes
 *****************************************************************************/

/* emonLibCM_defaults returns a default configuration (50 Hz mains) with:
 *  - interleaved I/V values
 * - 5 current channels
 * - 1 voltage channel
 * - 10 s reporting period
 */
struct ELC_CONFIG emonLibCM_defaults();

/*! \brief Configure the emonLibCM_core functions. Returns an ELC_STATUS struct
 *
 * @param [in] pCfg Pointer to ELC_CONFIG struct containing the configuration
 */
ELC_STATUS_t emonLibCM_init(const struct ELC_CONFIG * const pCfg);

/*! \brief Enable emonLibCM_core processing
 */
ELC_STATUS_t emonLibCM_enable();

/*! \brief Processes the partial sample set into the collecting accumulator
 */
void emonLibCM_process_sample();

/*! \brief At the end of a cycle, this flips collecting/processing buffers
 *         and clears the new collection buffer. Returns a pointer to the
 *         buffer for processing.
 */
struct Accumulator *emonLibCM_cycle_complete();


/*! \brief Process accumulated V/CT values into floating point RMS V/I, real
 *         power and energy
 * @param [in] pAcc Pointer to the raw accumulated values
 * @param [in] pRes Pointer to the floating point results
 */
void emonLibCM_process_float(const struct Accumulator *const pAcc, struct ELC_result_float *const pRes);

#endif
