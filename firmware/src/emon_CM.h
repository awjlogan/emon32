#ifndef EMON_CM_H
#define EMON_CM_H

#include "board_def.h"
#include "emon32.h"

/******************************************************************************
 * Type definitions
 *****************************************************************************/

typedef enum {
    INIT_SUCCESS,           // Init was successful
    INIT_FAIL_ENABLED,      // Init failed as currently enabled
    ENABLE_SUCCESS,
    ENABLE_FAIL_ENABLED,
} ECM_STATUS_t;

typedef struct {
    unsigned int reportCycles;  /* Number of cycles before reporting */
} ECMConfig_t;

typedef enum {
    POL_POS,
    POL_NEG
} Polarity_t;

typedef struct {
    uint32_t    sumV_sqr;
    int32_t     sumV_deltas;
} VAccumulator_t;

typedef struct {
    uint32_t    sumPA;
    uint32_t    sumPB;
    uint32_t    sumI_sqr;
    int32_t     sumI_deltas;
} CTAccumulator_t;

typedef struct {
    VAccumulator_t     processV[NUM_V];
    CTAccumulator_t    processCT[NUM_CT];
    unsigned int       num_samples;
} Accumulator_t;

/* This struct matches emonLibCM's calculations */
typedef struct {
    int16_t rmsCT;
    int16_t realPower;
    int16_t apparentPower;
    int16_t powerNow;
    int16_t powerFactor;
    int16_t wattHours;
    int16_t residualEnergy;
} CycleCT_t;


typedef struct {
    int16_t     rmsV[NUM_V];
    CycleCT_t   valCT[NUM_CT];
} ECMCycle_t;

typedef struct {
    int16_t realPower;
    uint16_t wattHour;
} DataCT_t;

/* KEY:VALUE pair to match EmonESP and EmonTx3eInterfacer */
typedef struct {
    uint32_t    msgNum;
    int16_t     rmsV[NUM_V];
    DataCT_t    CT[NUM_CT];
} ECMSet_t;

/******************************************************************************
 * Function prototypes
 *****************************************************************************/

/*! @brief Configure the power/energy accumulation system
 *  @param [in] pConfig : pointer to configuration
 */
void ecmInit(Emon32Config_t * const pConfig);

/*! @brief Swaps the ADC data buffer pointers
 */
void ecmSwapDataBuffer();

/*! @brief Returns a pointer to the ADC data buffer
 */
volatile RawSampleSetPacked_t *ecmDataBuffer();

/*! @brief For testing only, this function halves the incoming data rate with
 *         optional low pass filtering.
 *  @param [in] pDst : pointer to the SampleSet struct
 */
void ecmFilterSample(SampleSet_t *pDst);

/*! @brief Injects a raw sample from the ADC into the accumulators.
 */
void ecmInjectSample();

/*! @brief Processes a whole cycle
 */
void ecmProcessCycle();

/*! @brief Processes a whole data set
 *  @param [in] pSet : pointer to the processed data structure
 */
void ecmProcessSet(ECMSet_t *set);

#endif
