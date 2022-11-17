#ifndef DRIVER_DMAC_H
#define DRIVER_DMAC_H

#include "samd10.h"

/* @brief Setup the DMAC peripheral */
void dmacSetup();

/* @brief Returns a pointer to DMA descriptor for the channel.
 * @param [in] ch : channel
 */
volatile DmacDescriptor *dmacGetDescriptor(unsigned int ch);

/* @brief Start a transfer on DMAC channel
 * @param [in] ch : channel number
 */
void dmacStartTransfer(unsigned int ch);

#endif
