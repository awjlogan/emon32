#include "emonLibCM_compat.h"

typedef enum {
    RF12_868MHz,
    RF12_915MHz,
    RF12_433MHz
} RF_FREQ_t;

void rfm_init(RF_FREQ_t RF_freq);
void rfm_sendPayload(PayloadTx_t *pSrc, uint8_t group, uint8_t node, uint8_t whitening);
