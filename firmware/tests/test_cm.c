#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "emon_CM.h"

#define SAMPLE_RATE     4800u
#define MAINS_FREQ      50u
#define SMP_PER_CYCLE   SAMPLE_RATE / MAINS_FREQ

int
main(int argc, char *argv[])
{
    printf("---- emon32 CM test ----\n\n");

    const int16_t coeffLut[10] = {
        92, -279, 957, -2670, 10113, 10113, -2670, 957, -279, 92
    };

    volatile RawSampleSetPacked_t *volatile smpRaw;
    SampleSet_t                             smpProc;

    int16_t sine_q11[SMP_PER_CYCLE+1]; /* Need an extra point for 1 cycle */

    memset(&smpProc, 0, sizeof(SampleSet_t));

    printf("  Half band filter tests:\n");
    /* ecmDataBuffer returns a pointer to the buffer which the DMA is putting
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
    printf("    - Impulse: ");
    smpRaw->samples[0].smp[0] = 0;
    smpRaw->samples[1].smp[0] = INT16_MAX;
    ecmFilterSample(&smpProc);
    assert(coeffLut[0] == smpProc.smpV[0]);
    smpRaw->samples[0].smp[0] = 0;
    smpRaw->samples[1].smp[0] = 0;
    for (unsigned int idxCoeff = 0; idxCoeff < 9u; idxCoeff++)
    {
        ecmFilterSample(&smpProc);
        assert(coeffLut[idxCoeff + 1u] == smpProc.smpV[0]);
    }
    printf("Complete\n\n");
    exit(0);
    /* STEP TEST */
    /* SINE TEST */


    /* Generate a Q1.11 sine wave and use the smpRaw buffer to inject this
     * into the ecmInjectSample routine.
     */
    printf("  Inject sample test:\n");
    printf("    - Number of samples per cycle (2f): %d\n", SMP_PER_CYCLE);
    for (unsigned int i = 0; i < SMP_PER_CYCLE+1; i++)
    {
        double i_fp = (double)i;
        double phi = 2 * M_PI * i_fp / 96.0;
        double sine = sin(phi);
        sine_q11[i] = (int16_t)(sine * 2048.0);
    }

    unsigned int smpCnt = 0;
    do
    {
        for (unsigned int j = 0; j < VCT_TOTAL; j++)
        {
            smpRaw->samples[0].smp[j] = sine_q11[smpCnt];
            smpRaw->samples[1].smp[j] = sine_q11[smpCnt + 1];
        }

        /* Two physical samples are injected each time */
        smpCnt += 2u;
        if (smpCnt > SMP_PER_CYCLE) smpCnt = 0;

    } while (ECM_CYCLE_COMPLETE != ecmInjectSample());
}
