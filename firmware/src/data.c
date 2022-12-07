#include "data.h"

unsigned int
dataPackage(const ECMSet_t *pData, char *pDst, unsigned int n)
{
    unsigned int charCnt = 0;
    char tmpBuf[16];
    unsigned int cursor;
    unsigned int insLen;

    /* Message number */
    cursor = utilStrInsert(pDst, "MSG:", 0, 4);
    insLen = utilItoa(tmpBuf, pData->msgNum, ITOA_BASE10) - 1u;
    cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);

    /* V RMS for each channel.
     * TODO should this be a float?
     * TODO : how should this look for multiple V channels?
     */
    cursor = utilStrInsert(pDst, ",Vrms:", cursor, 6);
    insLen = utilItoa(tmpBuf, pData->rmsV[0], ITOA_BASE10) - 1u;
    cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);

    /* CT channels */
    for (unsigned int idxCT = 0; idxCT < NUM_CT; idxCT++)
    {
        cursor = utilStrInsert(pDst, ",P", cursor, 2);
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ":", cursor, 1);
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].realPower, ITOA_BASE10) - 1u;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ",E", cursor, 2);
        insLen = utilItoa(tmpBuf, (idxCT + 1u), ITOA_BASE10) - 1u;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
        cursor = utilStrInsert(pDst, ":", cursor, 1);
        insLen = utilItoa(tmpBuf, pData->CT[idxCT].wattHour, ITOA_BASE10) - 1u;
        cursor = utilStrInsert(pDst, tmpBuf, cursor, insLen);
    }

    /* TODO : temperature and pulse count are not implemented */

    /* Append NULL termination */
    cursor = utilStrInsert(pDst, "\0", cursor, 1);

    return 0;
}