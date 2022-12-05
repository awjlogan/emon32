#include "util.h"

unsigned int
utilStrlen(char *pBuf)
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
    for (unsigned int idx = 0; idx < ((len - 1) / 2); idx++)
    {
        tmp = pBuf[idx];
        pBuf[idx] = pBuf[idxEnd];
        pBuf[idxEnd] = tmp;
        idxEnd--;
    }
}

unsigned int
utilItoa(char *pBuf, int32_t val)
{
    unsigned int charCnt = 0;
    unsigned int isNegative;

    /* Handle 0 explicitly */
    if (0 == val)
    {
        *pBuf++ = '0';
        *pBuf = '\0';
        return 2u;
    }

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

    /* Append negative sign */
    if (isNegative)
    {
        *pBuf++ = '-';
        charCnt++;
    }

    /* Terminate and return */
    *pBuf = '\0';
    charCnt++;

    utilStrReverse(pBuf, charCnt - 1u);
    return charCnt;
}

void
utilStrInsert(char *pBuf, const char *pIns, unsigned int pos, unsigned int len)
{
}
