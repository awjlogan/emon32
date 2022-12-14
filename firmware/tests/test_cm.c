#include <stdio.h>
#include <math.h>
#include <string.h>

#include "emon_CM.h"

#define SAMPLE_RATE     4800u
#define MAINS_FREQ      50u
#define SMP_PER_CYCLE   SAMPLE_RATE / MAINS_FREQ

int
main(int argc, char *argv[])
{
    printf("---- emon32 CM test ----\n\n");

    volatile RawSampleSetPacked_t *volatile smpRaw;
    SampleSet_t smpProc;

    memset(&smpProc, 0, sizeof(SampleSet_t));

    /* Generate sine waves into data buffer (as Q11 fixed point)
     * ecmDataBuffer returns a pointer to the buffer which the DMA is putting
     * data into. Acquire the buffer, then swap so the test has access to the
     * processing buffer
     */
    smpRaw = ecmDataBuffer();
    ecmSwapDataBuffer();

    /* Half band tests : https://dspguru.com/dsp/faqs/fir/implementation/ */


    /* IMPULSE TEST
     * Inject an implulse, should get all the coefficients (except middle) out
     * TODO parameterise this for any size of filter
     */
    printf("  Impulse test:\n\n");
    for (unsigned int i = 0; i < VCT_TOTAL; i++)
    {
        smpRaw->samples[0].smp[i] = 0;
        smpRaw->samples[1].smp[i] = INT16_MAX;
    }
    ecmFilterSample(&smpProc);
    printf("  0 ||");
    for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
    {
        printf(" V%d : %d |", idxV, smpProc.smpV[idxV]);
    }
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        printf(" CT%d : %d |", idxCT, smpProc.smpCT[idxCT]);
    }
    printf("\n");

    for (unsigned int i = 0; i < VCT_TOTAL; i++)
    {
        smpRaw->samples[0].smp[i] = 0;
        smpRaw->samples[1].smp[i] = 0;
    }
    for (unsigned int idxCoeff = 0; idxCoeff < 8u; idxCoeff++)
    {
        ecmFilterSample(&smpProc);
        printf("  %d ||", (idxCoeff + 1u));
        for (unsigned int idxV = 0; idxV < NUM_V; idxV++)
        {
            printf(" V%d : %d |", idxV, smpProc.smpV[idxV]);
        }
        for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
        {
            printf(" CT%d : %d |", idxCT, smpProc.smpCT[idxCT]);
        }
        printf("\n");
    }
    printf("\n  Complete\n\n");

    /* STEP TEST */
    /* SINE TEST */

}
