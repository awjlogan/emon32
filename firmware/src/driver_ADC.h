#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include <stdint.h>

/*! @brief Configure the ADC for the board
 *  TODO Make this a configurable function
 */
void adcSetup();

/*! @brief Sets the destination address for the DMA transfer from the ADC
 *  @param [in] : address of the "collecting" structure
 */
void adcSetDestination(uint32_t buf);

#endif
