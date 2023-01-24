#include "data.h"

unsigned int
dataPackage_n(const ECMSet_t *pData, char *pDst, unsigned int n)
{
    unsigned int charCnt = 0;
    char tmpBuf[16];
    unsigned int cursor = 0u;
    unsigned int insLen;

    /* Message number */
    charCnt += 4;
    if (charCnt <= n)
    {
        cursor = utilStrInsert(pDst, "MSG:", 0, 4);
    }
    insLen = utilItoa(tmpBuf, pData->msgNum, ITOA_BASE10) - 1u;
    charCnt += insLen;
    if (charCnt <= n)
    {
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
    }

    /* V RMS for each channel.
     * TODO : how should this look for multiple V channels?
     */
    charCnt += 6;
    if (charCnt <= n)
    {
        cursor = utilStrInsert(pDst, ",Vrms:", cursor, 6);
    }
    insLen = utilFtoa(tmpBuf, pData->rmsV[0]) - 1u;
    charCnt += insLen;
    if (charCnt <= n)
    {
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
    }

    /* CT channels */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        charCnt += 2;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, ",P", cursor, 2);
        }
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        charCnt += insLen;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        }
        charCnt += 1;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, ":", cursor, 1);
        }
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].realPower, ITOA_BASE10) - 1u;
        charCnt += insLen;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        }
        charCnt += 2;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, ",E", cursor, 2);
        }
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        charCnt += insLen;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        }
        charCnt += 1;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, ":", cursor, 1);
        }
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].wattHour, ITOA_BASE10) - 1u;
        charCnt += insLen;
        if (charCnt <= n)
        {
            cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        }
    }

    /* TODO : temperature and pulse count are not implemented */

    return charCnt;
}

unsigned int
dataPackage(const ECMSet_t *pData, char *pDst)
{
    unsigned int    charCnt;
    char            tmpBuf[16];
    unsigned int    cursor;
    unsigned int    insLen;

    /* Message number */
    cursor = utilStrInsert(pDst, "MSG:", 0, 4);
    charCnt = 4u;
    insLen = utilItoa(tmpBuf, pData->msgNum, ITOA_BASE10) - 1u;
    charCnt += insLen;
    cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);

    /* V RMS for each channel.
     * TODO : how should this look for multiple V channels?
     */
    cursor = utilStrInsert(pDst, ",Vrms:", cursor, 6);
    charCnt += 6u;
    insLen = utilFtoa(tmpBuf, (pData->rmsV[0] * 0.01f)) - 1u;
    charCnt += insLen;
    cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);

    /* CT channels */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        cursor = utilStrInsert(pDst, ",P", cursor, 2);
        charCnt += 2u;
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        charCnt += insLen;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ":", cursor, 1);
        charCnt += 1u;
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].realPower, ITOA_BASE10) - 1u;
        charCnt += insLen;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ",E", cursor, 2);
        charCnt += 2u;
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        charCnt += insLen;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ":", cursor, 1);
        charCnt += 1u;
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].wattHour, ITOA_BASE10) - 1u;
        charCnt += insLen;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
    }

    /* TODO : temperature and pulse count are not implemented */
    return charCnt;
}
