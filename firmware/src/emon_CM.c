#include "emon32_samd.h"
#include <string.h>

/*! @brief Swap pointers to buffers
 */
static void
ecmSwapPtr(void *pIn1, void *pIn2)
{
    void *tmp = pIn1;
    pIn1 = pIn2;
    pIn2 = tmp;
}

/******************************************************************************
 * Data acquisition
 *****************************************************************************/

static volatile SampleSetPacked_t adc_samples[SAMPLE_BUF_DEPTH];
static volatile SampleSetPacked_t *volatile adc_active = adc_samples;
static volatile SampleSetPacked_t *volatile adc_proc = adc_samples + 1;

void
ecmSwapDataBuffer()
{
    ecmSwapPtr((void *)adc_active, (void *)adc_proc);
}

volatile SampleSetPacked_t *
ecmDataBuffer()
{
    return adc_active;
}

/******************************************************************************
 * Pre-processing
 *****************************************************************************/

static SampleSet_t      samples[PROC_DEPTH];

/******************************************************************************
 * Power accumulators
 *****************************************************************************/

static Accumulator_t    accum_buffer[2];
static Accumulator_t *  accum_collecting = accum_buffer;
static Accumulator_t *  accum_processing = accum_buffer + 1;

/******************************************************************************
 * Functions
 *****************************************************************************/

static void
ecmUnpackSample(SampleSet_t * pDst)
{
#ifndef DOWNSAMPLE_DSP
    /* No filtering, discard the second sample in the set */
    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        pDst->smpV[idxV] = adc_proc->samples[0].smpV[idxV];
    }

    for (unsigned int idxI = 0; idxI < NUM_CT; idxI++)
    {
        pDst->smpCT[idxI] = adc_proc->samples[0].smpCT[idxI];
    }
#else
    /* TODO Make half band filter */
#endif
}

void
ecmInjectSample()
{
    SampleSet_t smpProc;
    SampleSet_t *pSmpProc = &smpProc;
    static unsigned int idxInject;

    /* Zero crossing detection
     * TODO Should this be a hardware based system, if enough pins? */
    Polarity_t polarity_now;
    static Polarity_t polarity_last = POL_POS;
    static int hystCnt = 0;

    /* Copy the pre-processed sample data into the ring buffer */
    ecmUnpackSample(pSmpProc);
    memcpy((void *)(samples + idxInject), (const void *)pSmpProc, sizeof(SampleSet_t));

    /* Do power calculations */
    accum_collecting->num_samples++;

    /* TODO this is only for single phase currently, loop is there for later */
    const int32_t thisV = (int32_t)samples[idxInject].smpV[0];

    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        accum_collecting->processV[idxV].sumV_sqr += thisV * thisV;
        accum_collecting->processV[idxV].sumV_deltas += thisV;
    }

    const unsigned int idxLast = (idxInject - 1u) & (PROC_DEPTH - 1u);
    const int16_t lastV = samples[idxLast].smpV[0];

    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        const int32_t lastCT = (int32_t)samples[idxLast].smpCT[idxCT];
        accum_collecting->processCT[idxCT].sumPA += lastCT * lastV;
        accum_collecting->processCT[idxCT].sumPB += lastCT * thisV;
        accum_collecting->processCT[idxCT].sumI_sqr += lastCT * lastCT;
        accum_collecting->processCT[idxCT].sumI_deltas += lastCT;
    }

    /* Zero crossing detection. Flag for processing if cycle is complete */
    polarity_now = thisV >= 0 ? POL_POS : POL_NEG;
    if (polarity_now != polarity_last)
    {
        hystCnt++;
        if (2u == hystCnt)
        {
            polarity_last = polarity_now;
            hystCnt = 0u;
            if (POL_POS == polarity_now)
            {
                emon32SetEvent(EVT_ECM_CYCLE_CMPL);
                ecmSwapPtr((void *)accum_collecting, (void *)accum_processing);
                memset((void *)accum_collecting, 0, sizeof(Accumulator_t));
            }
        }
    }
    /* Advance injection point, masking for overflow */
    idxInject = (idxInject + 1u) & (PROC_DEPTH - 1u);
}

void
ecmProcessCycle()
{
    static unsigned int cycleCount;

    cycleCount++;
    if (cycleCount >= 500)
    {
        cycleCount = 0u;
        emon32SetEvent(EVT_ECM_SET_CMPL);
    }
}

void
ecmProcessSet(ECMSet_t *set)
{
}
