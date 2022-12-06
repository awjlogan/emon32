#include "util.h"

unsigned int
utilStrlen(const char *pBuf)
{
    unsigned int charCnt = 0;
    while (*pBuf++)
    {
        charCnt++;
    }
    return charCnt;
}

void
utilStrReverse(char *pBuf, unsigned int len)
{
    char tmp;
    unsigned int idxEnd = len - 1u;
    for (unsigned int idx = 0; idx < (len / 2); idx++)
    {
        tmp = pBuf[idx];
        pBuf[idx] = pBuf[idxEnd];
        pBuf[idxEnd] = tmp;
        idxEnd--;
    }
}

unsigned int
utilItoa(char *pBuf, int32_t val, ITOA_BASE_t base)
{
    unsigned int charCnt = 0;
    unsigned int isNegative = 0;
    char * const pBase = pBuf;

    /* Handle 0 explicitly */
    if (0 == val)
    {
        *pBuf++ = '0';
        *pBuf = '\0';
        return 2u;
    }

    /* Base 10 can be signed, and has a divide in */
    if (ITOA_BASE10 == base)
    {
        if (val < 0)
        {
            isNegative = 1u;
            val = -val;
        }

        while (0 != val)
        {
            *pBuf++ = (val % 10u) + '0';
            val = val / 10u;
            charCnt++;
        }

        if (0 != isNegative)
        {
            *pBuf++ = '-';
            charCnt++;
        }
    }
    else
    {
        const char itohex[] = "0123456789abcdef";
        uint32_t val_u = (uint32_t)val;

        while (0 != val_u)
        {
            *pBuf++ = itohex[(val_u & 0xFu)];
            val_u >>= 4;
            charCnt++;
        }
    }

    /* Terminate and return */
    *pBuf = '\0';
    charCnt++;

    utilStrReverse(pBase, charCnt - 1u);
    return charCnt;
}

unsigned int
utilStrInsert(char *pDst, const char *pIns, unsigned int pos, unsigned int len)
{
    char *pOffset = pDst + pos;
    for (unsigned int cntIns = 0; cntIns < len; cntIns++)
    {
        *pOffset++ = *pIns++;
    }
    return (pos + len);
}
