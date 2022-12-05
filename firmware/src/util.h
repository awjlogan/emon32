#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

/*! @brief Returns the number of characters up to, but not including, NULL
 *  @param [in] pBuf : pointer to the string buffer
 */
unsigned int utilStrlen(char *pBuf);

/*! @brief Convert integer to null terminated base 10 string. Returns the
 *         number of characters (including NULL).
 *  @param [in] pBuf : pointer to string buffer, at least 11 characters
 *  @param [in] val : value to convert
 */
unsigned int utilItoa(char *pBuf, int32_t val);


/*! @brief Insert a string into an existing buffer. Note, this does not guard
 *         against overrunning the buffer!
 *  @param [in] pBuf : pointer to the buffer to insert into
 *  @param [in] pIns : pointer to the string to insert
 *  @param [in] pos  : start position to insert
 *  @param [in] len  : length of string to insert
 */
void utilStrInsert(char *pBuf, const char *pIns, unsigned int pos, unsigned int len);

#endif
