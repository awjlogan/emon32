#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

/*! @brief Save data to EEPROM. All writes are contigyous from the bae
 *         address. The implementation should account for page sizes.
 *  @param [in] addr : base address
 *  @param [in] pSrc : pointer to data
 *  @param [in] n    : number of bytes to send
 */
void eepromWrite(unsigned int addr, const void *pSrc, unsigned int n);

#endif