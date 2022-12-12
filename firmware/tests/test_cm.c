#include <stdio.h>
#include <math.h>

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

    /* Generate sine waves into data buffer (as Q11 fixed point)
     * ecmDataBuffer returns a pointer to the buffer which the DMA is putting
     * data into. Acquire the buffer, then swap so the test has access to the
     * processing buffer
     */
    smpRaw = ecmDataBuffer();
    ecmSwapDataBuffer();

    for (unsigned int idx = 0; idx < SMP_PER_CYCLE; idx++)
    {
        printf("%f ", ((double)idx * 2 * M_PI) / 96.0);
    }
    printf("\n");

    /* Half band tests : https://dspguru.com/dsp/faqs/fir/implementation/ */
    /* IMPULSE TEST */
    /* STEP TEST */
    /* SINE TEST */

}
