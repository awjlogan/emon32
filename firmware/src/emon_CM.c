#include "emonLibCM_core.h"

/******************************************************************************
 * Local variables
 *****************************************************************************/

static bool libcm_enabled = false;
static struct ELC_CONFIG elc_config;

static struct Accumulator bufferA;
static struct Accumulator bufferB;
static struct Accumulator *pCollect = &bufferA;
static struct Accumulator *pProcess = &bufferB;

/******************************************************************************
 * Functions
 *****************************************************************************/

struct ELC_CONFIG
emonLibCM_defaults()
{
    struct ELC_CONFIG default_config;
    default_config.num_report_cycles  = 500ul;    // (50 Hz * 10 s)
    default_config.depth_ADC_raw = 4u;
    default_config.pADC_raw = 0u;
    return default_config;
}

ELC_STATUS_t
emonLibCM_init(const struct ELC_CONFIG * const pCfg)
{
    /* Can not initialise while enabled */
    if (libcm_enabled) return INIT_FAIL_ENABLED;

    /* Revisit : sanitise configuration */
    elc_config = *pCfg;
    return INIT_SUCCESS;
}

ELC_STATUS_t
emonLibCM_enable()
{
    if (libcm_enabled) return ENABLE_FAIL_ENABLED;

    libcm_enabled = true;
    return ENABLE_SUCCESS;
}

void
emonLibCM_process_sample()
{
    /* Revisit: currently only handles one voltage channel */

    /* Current sample index */
    static uint8_t smpIdx;
    /* Previous sample index - unsigned integer wraps
     * Revisit: configurable mask
     */
    const uint8_t smpIdx_prev = (smpIdx - 1u) & 0x3u;

    const int32_t sampleV = (int32_t)elc_config.pADC_raw[smpIdx].adcV[0];
    const int32_t sampleV_last = (int32_t)elc_config.pADC_raw[smpIdx_prev].adcV[0];

    pCollect->processV[0].sumV_sqr = sampleV * sampleV;
    pCollect->processV[0].sumV_deltas += sampleV;

    for (size_t idxCT = 0; idxCT < NUM_CHAN_CT; idxCT++)
    {
        const int32_t sampleI = (int32_t)elc_config.pADC_raw[smpIdx_prev].adcCT[idxCT];
        pCollect->processI[idxCT].sumI_sqr = sampleI * sampleI;
        pCollect->processI[idxCT].sumI_deltas += sampleI;
    }
    smpIdx++;
    pCollect->num_samples++;
}

struct Accumulator *
emonLibCM_cycle_complete()
{
    /* Swap processing and collection buffers */
    const struct Accumulator accZero = {};
    struct Accumulator *const accSwap = pCollect;
    pCollect = pProcess;
    pProcess = accSwap;
    /* Clear new collection buffer */
    *pCollect = accZero;
    return pProcess;
}

void
emonLibCM_cycle_float(const struct Accumulator *const pAcc, struct ELC_result_float *const pRes)
{
    /* Roll over any partial Wh from previous cycle */
    static float residualEnergy[NUM_CHAN_CT];

    /* Voltage channel
     * Revisit : handle multiple V channels
     */
    pRes->rmsV[0] = 0.0;

    /* CT channels */
    for (uint8_t idxCT = 0; idxCT < NUM_CHAN_CT; idxCT)
    {
        const float sumRealPower = 0.0;
        const float powerNow = sumRealPower;
        pRes->resultCT[idxCT].rmsCT = 0.0;

        const float VA = pRes->resultCT[idxCT].rmsCT * pRes->rmsV[0];
        pRes->resultCT[idxCT].powerFactor = powerNow / VA;
        pRes->resultCT[idxCT].realPower = powerNow + 0.5;
        const float energyNow = powerNow * pAcc->num_samples + residualEnergy[idxCT];
    }
}
