#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

typedef struct {
    uint16_t    addr_base;      /* Block base address */
    int8_t      idxNextWrite;   /* Index of next write to EEPROM */
    uint8_t     blkCnt;         /* Number of available blocks */
    uint8_t     dataSize;       /* Size (bytes) of the data */
    void *      pData;          /* Pointer to the packed data */
} eepromPktWL_t;

/*! @brief Save data to EEPROM. All writes are contiguous from the base
 *         address. The implementation should account for page boundaries.
 *  @param [in] addr : base address
 *  @param [in] pSrc : pointer to data
 *  @param [in] n    : number of bytes to send
 */
void eepromWrite(unsigned int addr, const void *pSrc, unsigned int n);

/*! @brief Save data to EEPROM with wear leveling.
 *  @param [in] pPktWr : pointer to write packet
 */
void eepromWriteWL(eepromPktWL_t *pPktWr);

/*! @brief Read data from EEPROM with wear leveling
 *  @param [in] pPktRd : pointer to read packet
 */
void eepromReadWL(eepromPktWL_t *pPktRd);

#endif
