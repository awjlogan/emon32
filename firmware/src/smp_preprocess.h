#ifndef SMP_PREPROCESS_H
#define SMP_PREPROCESS_H

#include "emon32.h"

/*! @brief Unpack the packed struct used for the DMA transfer. This allows
 *         usage of the data without requiring byte-wise access
 * @param [in] : pSrc pointer to the packed struct of samples
 * @param [in] : pDst pointer to the unpacked struct of samples
 */

void preUnpackSample(const volatile SampleSetPacked_t *const pSrc, SampleSet_t *const pDst);

#endif
