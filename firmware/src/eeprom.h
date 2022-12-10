#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

/*! @brief Save a byte to an address
 *  @param [in] addr : address to write to
 *  @param [in] data : byte to write
 */
void eepromWriteByte(unsigned int addr, uint8_t data);

/*! @brief Save an 8 byte page
 *  @param [in] addr : base address
 *  @param [in] pData : pointer to 8 bytes of data
 */
void eepromWritePage8(unsigned int addr, const uint8_t *pData);

/*! @brief Save a 16 byte page
 *  @param [in] addr : base address
 *  @param [in] pData : pointer to 16 bytes of data
 */
void eepromWritePage16(unsigned int addr, const uint8_t *pData);

#endif