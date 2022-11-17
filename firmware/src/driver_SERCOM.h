#ifndef DRIVER_SERCOM_H
#define DRIVER_SERCOM_H

#include <stdint.h>

typedef struct SERCOM_CFG {
    // TODO Config details
} SERCOM_t;

/* @brief configure the serial communication modules */
void sercomSetup();

/* @brief Send a single character (blocking) on UART
 * @param [in] Single character
 */

void uartPutcBlocking(char c);

/* @brief Send a string (blocking) on UART
 * @param [in] Pointer to null terminated string
 */
void uartPutsBlocking(const char *s);

/* @brief Configure the DMA for non-blocking transactions
 */
void uartConfigureDMA();

/* @brief Send a string (non-blocking) on UART by DMA
 * @param [in] Pointer to the string
 * @param [in] Length of the string (not including NULL)
 */
void uartPutsNonBlocking(const char * const s, uint16_t len);

#endif
