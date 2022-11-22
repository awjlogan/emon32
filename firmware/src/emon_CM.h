#ifndef EMON_CM_H
#define EMON_CM_H

#include "emon32.h"
#include "board_def.h"

/******************************************************************************
 * Type definitions
 *****************************************************************************/

typedef enum {
    INIT_SUCCESS,           // Init was successful
    INIT_FAIL_ENABLED,      // Init failed as currently enabled
    ENABLE_SUCCESS,
    ENABLE_FAIL_ENABLED,
} ECM_STATUS_t;


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
    struct VAccumulator     processV[NUM_V];
    struct CTAccumulator    processI[NUM_CT];
    uint8_t                 num_samples;
};

struct ECM_result_CT_float {
    float rmsCT;
    float powerFactor;
    float realPower;
    float wattHour;
};

struct ECM_result_float {
    float                       rmsV[NUM_V];
    struct ECM_result_CT_float  resultCT[NUM_CT];
};

struct ECM_CONFIG {
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

/*! @brief Configure the power/energy accumulation system */

/*! @brief Injects a raw sample from the ADC into the accumulators.
 *  @param [in] : pointer to a struct containing the V/CT values with the DC
 *                bias (roughly) removed
 */
void ecmInjectSample(const volatile SampleSet_t *const smp);

/* cm_defaults returns a default configuration (50 Hz mains) with:
 *  - interleaved I/V values
 * - 5 current channels
 * - 1 voltage channel
 * - 10 s reporting period
 */
struct ECM_CONFIG cm_defaults();

/*! \brief Configure the cm_core functions. Returns an ECM_STATUS struct
 * @param [in] pCfg Pointer to ECM_CONFIG struct containing the configuration
 */
ECM_STATUS_t cm_init(const struct ECM_CONFIG * const pCfg);

/*! \brief Enable cm_core processing
 */
ECM_STATUS_t cm_enable();

/*! \brief Processes the partial sample set into the collecting accumulator
 */
void cm_process_sample();

/*! \brief At the end of a cycle, this flips collecting/processing buffers
 *         and clears the new collection buffer. Returns a pointer to the
 *         buffer for processing.
 */
struct Accumulator *cm_cycle_complete();


/*! \brief Process accumulated V/CT values into floating point RMS V/I, real
 *         power and energy
 * @param [in] pAcc Pointer to the raw accumulated values
 * @param [in] pRes Pointer to the floating point results
 */
void cm_process_float(const struct Accumulator *const pAcc, struct ECM_result_float *const pRes);

#endif
