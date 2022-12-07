// #include "emon32_samd.h"
#include "emon_CM.h"
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
 * Configuration
 *****************************************************************************/

static ECMConfig_t ecmConfig = {.reportCycles = 50u};

ECMConfig_t *
ecmGetConfig()
{
    return &ecmConfig;
}

void
ecmInit()
{
}

/******************************************************************************
 * Data acquisition
 *****************************************************************************/

static volatile RawSampleSetPacked_t adc_samples[SAMPLE_BUF_DEPTH];
static volatile RawSampleSetPacked_t *volatile adc_active = adc_samples;
static volatile RawSampleSetPacked_t *volatile adc_proc = adc_samples + 1;

void
ecmSwapDataBuffer()
{
    ecmSwapPtr((void *)adc_active, (void *)adc_proc);
}

volatile RawSampleSetPacked_t *
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

/*! @brief Unpack and optionally low pass filter the raw sample
 *         The struct from the DMA has no partition into V/CT channels, so
 *         alter this function to move data from the implementation specific
 *         DMA addressess to the defined SampleSet_t fields
 * @param [in] pDst : pointer to the SampleSet_t destination
 */
static void
ecmUnpackSample(SampleSet_t *pDst)
{
#ifndef DOWNSAMPLE_DSP

    /* No filtering, discard the second sample in the set */
    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        pDst->smpV[idxV] = adc_proc->samples[0].smp[idxV];
    }

    for (unsigned int idxCT = NUM_V; idxCT < (NUM_CT + NUM_V); idxCT++)
    {
        pDst->smpCT[idxCT] = adc_proc->samples[0].smp[idxCT];
    }

#else

    /* The FIR half band filter is symmetric, so the coefficients are folded.
     * Alternating coefficients are 0, so are not included in any outputs.
     * For an ODD number of taps, the centre coefficent is handled
     * individually, then the other taps in a loop.
     *
     * b_0 | b_2 | .. | b_X | .. | b_2 | b_0
     *
     * For an EVEN number of taps, loop across all the coefficients:
     *
     * b_0 | b_2 | .. | b_2 | b_0
     */

    static unsigned int idxInj = 0u;
    static RawSampleSetUnpacked_t smpBuffer[DOWNSAMPLE_TAPS];
    int32_t intRes[VCT_TOTAL] = {0};
    const int16_t firCoeffs[DOWNSAMPLE_TAPS / 2] =
    {
            0x0001, 0x0010, 0x0011, 0x0100, 0x0101, 0x0110, 0x0111, 0x1000,
            0x1001, 0x1010, 0x1011, 0x1100, 0x1101, 0x1110, 0x1111, 0x2000
    };


    const unsigned int idxInjPrev =   (0 == idxInj)
                                    ? (DOWNSAMPLE_TAPS - 1u)
                                    : (idxInj - 1u);

    /* Copy the packed raw ADC value into the unpacked buffer */
    for (unsigned int idxSmp = 0; idxSmp < (NUM_V + NUM_CT); idxSmp++)
    {
        smpBuffer[idxInj].smp[idxSmp] = adc_proc->samples[1].smp[idxSmp];
        smpBuffer[idxInjPrev].smp[idxSmp] = adc_proc->samples[0].smp[idxSmp];
    }

    /* For an ODD number of taps, take the unique middle value to start */
    if (0 != DOWNSAMPLE_TAPS % 2)
    {
        const unsigned int idxSub = (DOWNSAMPLE_TAPS / 2) - 1;
        const int16_t coeff = firCoeffs[idxSub];
        int idxSmp = idxInj - (DOWNSAMPLE_TAPS / 2);
        if (idxSmp < 0)
        {
            idxSmp = DOWNSAMPLE_TAPS - (DOWNSAMPLE_TAPS / 2) + idxInj;
        }

        for (unsigned int idxChannel = 0; idxChannel < VCT_TOTAL; idxChannel++)
        {
            intRes[idxChannel] += coeff * smpBuffer[idxSmp].smp[idxChannel];
        }
    }

    /* Loop over the FIR coefficients, sub loop through channels. The filter
     * is folded so the symmetric FIR coefficients are used for both samples.
     */
    unsigned int idxSmpStart = idxInj;
    int idxSmpEnd = idxInj - (DOWNSAMPLE_TAPS / 2);
    if (idxSmpEnd < 0)
    {
        idxSmpEnd = DOWNSAMPLE_TAPS - (DOWNSAMPLE_TAPS / 2) + idxInj;
    }

    for (unsigned int idxCoeff = 0; idxCoeff < DOWNSAMPLE_TAPS / 2; idxCoeff++)
    {
        const int16_t coeff = firCoeffs[idxCoeff];
        for (unsigned int idxChannel = 0; idxChannel < VCT_TOTAL; idxChannel++)
        {
            intRes[idxChannel] +=   coeff
                                  * (  smpBuffer[idxSmpStart].smp[idxChannel]
                                     + smpBuffer[idxSmpEnd].smp[idxChannel]);
        }

        /* Converge toward the middle, check for over/underflow */
        idxSmpStart =   ((DOWNSAMPLE_TAPS - 1) == idxSmpStart)
                       ? 0
                       : idxSmpStart + 1u;
        idxSmpEnd =   (0 == idxSmpEnd)
                    ? (DOWNSAMPLE_TAPS - 1)
                    : idxSmpEnd - 1u;
    }

    /* Truncate with rounding to nearest LSB and place into field */
    /* TODO This may not be a good way to separate V/CT channels */
    for (unsigned int idxChannel = 0; idxChannel < VCT_TOTAL; idxChannel++)
    {
        unsigned int roundUp = 0;
        if (0 != (intRes[idxChannel] & (1u << 15)))
        {
            roundUp = 1u;
        }
        intRes[idxChannel] = (intRes[idxChannel] >> 16) + roundUp;

        if (idxChannel < NUM_V)
        {
            pDst->smpV[idxChannel] = (uint16_t)intRes[idxChannel];
        }
        else
        {
            pDst->smpCT[idxChannel - NUM_V] = (int16_t)intRes[idxChannel];
        }

    }

    /* Each injection is 2 samples */
    idxInj += 2u;
    if (idxInj > (DOWNSAMPLE_TAPS - 1))
    {
        idxInj -= (DOWNSAMPLE_TAPS - 1);
    }

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
//     polarity_now = thisV >= 0 ? POL_POS : POL_NEG;
//     if (polarity_now != polarity_last)
//     {
//         hystCnt++;
//         if (2u == hystCnt)
//         {
//             polarity_last = polarity_now;
//             hystCnt = 0u;
//             if (POL_POS == polarity_now)
//             {
//                 emon32SetEvent(EVT_ECM_CYCLE_CMPL);
//                 ecmSwapPtr((void *)accum_collecting, (void *)accum_processing);
//                 memset((void *)accum_collecting, 0, sizeof(Accumulator_t));
//             }
//         }
//     }
    static uint8_t count = 0;
    count++;
    if (32 == count)
    {
        count = 0;
        // emon32SetEvent(EVT_ECM_CYCLE_CMPL);
        ecmSwapPtr((void *)accum_collecting, (void *)accum_processing);
        memset((void *)accum_collecting, 0, sizeof(Accumulator_t));
    }

    /* Advance injection point, masking for overflow */
    idxInject = (idxInject + 1u) & (PROC_DEPTH - 1u);
}

void
ecmProcessCycle()
{
    static unsigned int cycleCount;
    static volatile ECMCycle_t   ecmCycle;

    cycleCount++;
    /* Reused constants */
    const uint32_t numSamples = accum_processing->num_samples;
    const uint32_t numSamplesSqr = numSamples * numSamples;

    /* RMS for V channels */
    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        ecmCycle.rmsV[idxV] =   ((accum_processing->processV[idxV].sumV_sqr / numSamples)
                              - ((accum_processing->processV[idxV].sumV_deltas * accum_processing->processV[idxV].sumV_deltas) / (numSamplesSqr)));
    }

    /* RMS for CT channels */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        ecmCycle.valCT[idxCT].rmsCT =   ((accum_processing->processCT[idxCT].sumI_sqr / numSamples)
                                      - ((accum_processing->processCT[idxCT].sumI_deltas * accum_processing->processCT[idxCT].sumI_deltas) / numSamplesSqr));
    }
    if (cycleCount >= ecmConfig.reportCycles)
    {
        cycleCount = 0u;
        // emon32SetEvent(EVT_ECM_SET_CMPL);
    }
}

void
ecmProcessSet(ECMSet_t *set)
{
    set->msgNum++;
}
