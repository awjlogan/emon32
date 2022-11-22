#ifndef DRIVER_DMAC_H
#define DRIVER_DMAC_H

#include "samd10.h"

/*! @brief Setup the DMAC peripheral */
void dmacSetup();

/*! @brief Returns a pointer to DMA descriptor for the channel.
 *  @param [in] ch : channel
 */
volatile DmacDescriptor *dmacGetDescriptor(unsigned int ch);

/*! @brief Start a transfer on DMAC channel
 *  @param [in] ch : channel number
 */
void dmacStartTransfer(unsigned int ch);

/*! @brief Enable DMAC channel interrupt
 *  @param [in] ch : channel to enable interrupt for
 */
void dmacEnableChannelInterrupt(unsigned int ch);

/*! @brief Disable DMAC channel interrupt
 *  @param [in] ch : channel to disable interrupt for
 */
void dmacDisableChannelInterrupt(unsigned int ch);

/*! @brief Clear DMAC channel interrupt flag
 *  @param [in] ch : channel to clear interrupt flag for
 */
void dmacClearChannelInterrupt(unsigned int ch);

#endif
