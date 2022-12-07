#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "data.h"
#include "util.h"

int
main(int argc, char *argv[])
{
    ECMSet_t dataset = {};
    char txBuffer[1024];
    char goldBuffer[1024];
    unsigned int dataCnt;
    unsigned int goldCnt;

    printf("----- emon32 data test -----\n\n");
    printf("  Num V:  %d\n", NUM_V);
    printf("  Num CT: %d\n\n", NUM_CT);

    printf("  dataPackage ... ");

    dataCnt = dataPackage(&dataset, txBuffer, 1024);
    goldCnt = snprintf(goldBuffer, 1024, "MSG:%d,Vrms:%d,P1:%d,E1:%d,P2:%d,E2:%d,P3:%d,E3:%d,P4:%d,E4:%d",
                   dataset.msgNum, dataset.rmsV[0], dataset.CT[0].realPower, dataset.CT[0].wattHour, dataset.CT[1].realPower, dataset.CT[1].wattHour, dataset.CT[2].realPower, dataset.CT[2].wattHour, dataset.CT[3].realPower, dataset.CT[3].wattHour);
    if (0 != strcmp(txBuffer, goldBuffer) || dataCnt != goldCnt)
    {
        printf("  all data == 0:\n");
        printf("    %s\n", txBuffer);
        printf("    %d | %d\n", dataCnt, goldCnt);
        assert(0);
    }

    memset(&dataset, 'F', sizeof(ECMSet_t));
    dataCnt = dataPackage(&dataset, txBuffer, 1024);
    goldCnt = snprintf(goldBuffer, 1024, "MSG:%d,Vrms:%d,P1:%d,E1:%d,P2:%d,E2:%d,P3:%d,E3:%d,P4:%d,E4:%d",
                   dataset.msgNum, dataset.rmsV[0], dataset.CT[0].realPower, dataset.CT[0].wattHour, dataset.CT[1].realPower, dataset.CT[1].wattHour, dataset.CT[2].realPower, dataset.CT[2].wattHour, dataset.CT[3].realPower, dataset.CT[3].wattHour);
    if (0 != strcmp(txBuffer, goldBuffer) || dataCnt != goldCnt)
    {
        printf("  all data == MAX:\n");
        printf("    %s\n", txBuffer);
        printf("    %d | %d\n", dataCnt, goldCnt);
        assert(0);
    }

    printf("PASSED\n\n");

}