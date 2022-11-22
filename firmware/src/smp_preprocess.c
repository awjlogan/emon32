#include "emon32.h"

void
preUnpackSample(const volatile SampleSetPacked_t *const pSrc, SampleSet_t *const pDst)
{
#ifndef DOWNSAMPLE_DSP
    /* No filtering, discard the second sample in the set */
    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        pDst->smpV[idxV] = pSrc->samples[0].smpV[idxV];
    }

    for (unsigned int idxI = 0; idxI < NUM_CT; idxI++)
    {
        pDst->smpCT[idxI] = pSrc->samples[0].smpCT[idxI];
    }
#else
    /* TODO Make half band filter */
#endif
}
