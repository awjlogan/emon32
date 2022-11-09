#include "driver_RFM69.h"
#include "driver_SERCOM.h"
#include "emonLibCM_compat.h"

/* RFM69CW registers */
const uint8_t RFM_RegOpMode     = 0x01u;
const uint8_t RFM_RegDataModul  = 0x02u;
const uint8_t RFM_RegBitrateMsb = 0x03u;
const uint8_t RFM_RegBitrateLsb = 0x04u;
const uint8_t RFM_FdevMsb       = 0x05u;
const uint8_t RFM_FdevLsb       = 0x06u;
const uint8_t RFM_RegFrfMsb     = 0x07u;
const uint8_t RFM_RegFrfMid     = 0x08u;
const uint8_t RFM_RefFrfLsb     = 0x09u;
const uint8_t RFM_IRQFlags2     = 0x28u;
const uint8_t RFM_RegSyncValue1 = 0x2Fu;

/* RFM69CW bit masks */
const uint8_t REG_FIFO          = 0x00u;
const uint8_t REG_OPMODE        = 0x01u;
const uint8_t MODE_SLEEP        = 0x00u;
const uint8_t MODE_TRANSMITTER  = 0x0Cu;
const uint8_t MODE_RECEIVER     = 0x10u;
const uint8_t REG_DIOMAPPING1   = 0x25u;
const uint8_t REG_IRQFLAGS1     = 0x27u;
const uint8_t MODE_READY        = 0x80u;
const uint8_t IRQ2_FIFOFULL     = 0x80u;
const uint8_t IRQ2_FIFONOTEMPTY = 0x40u;
const uint8_t IRQ2_PACKETSENT   = 0x08u;
const uint8_t IRQ2_FIFOOVERRUN  = 0x10u;
const uint8_t REG_PACKET_CONFIG2= 0x3Du;
const uint8_t RESTART_RX        = 0x04u;
const uint8_t REG_RSSI_CONFIG   = 0x23u;
const uint8_t RSSI_START        = 0x01u;
const uint8_t RSSI_DONE         = 0x02u;
const uint8_t REG_RSSI_VALUE    = 0x24u;

/* @brief Copy of the avr-libc CRC16 routine. Returns the current CRC16
 *        The polynomial is 0xA001, initial value is 0xFFFF
 *        https://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
 * @param [in]  crc: The current CRC value
 * @param [in]  data: Data byte to add to CRC
 */
static inline uint16_t
crc16_update(uint16_t crc, uint8_t data)
{
    const uint16_t POLY = 0xA001u;
    crc ^= data;
    for (uint8_t i = 0; i < 8; ++i)
    {
        crc = (crc & 0x1u) ? (crc >> 1u) ^ POLY : (crc >> 1u);
    }
    return crc;
}

void spiWrite(uint8_t addr, uint8_t data)
{
}

uint8_t spiRead(uint8_t addr);

void
rfm_sleep()
{
    while (!(spiRead(RFM_IRQFlags2) & IRQ2_PACKETSENT));
        // TODO Add short delay
    spiWrite(RFM_RegOpMode, (spiRead(RFM_RegOpMode) & 0xE3u) | 0x01u);

void
rfm_init(RF_FREQ_t RFM_freq)
{
    // TODO Get a configuration struct from the SPI SERCOM, and set it up
    uint8_t readVal;

    spiWrite(RFM_RegSyncValue1, 0xAAu);
    while (0xAAu != spiRead(RFM_RegSyncValue1));
    spiWrite(RFM_RegSyncValue1, 0x55u);
    while (0x55u != spiRead(RFM_RegSyncValue1));

    spiWrite(RFM_RegOpMode, 0x04u);
    spiWrite(RFM_RegDataModul, 0x0u);
    spiWrite(RFM_RegBitrateMsb, 0x02u); // ~49.2 kbps
    spiWrite(RFM_RegBitrateLsb, 0x08u);

    // Select frequency
    if (RF12_868MHz == RFM_freq)
    {
        spiWrite(RFM_RegFrfMsb, 0xD9u);
        spiWrite(RFM_RegFrfMid, 0x00u);
    }
    else if (RF12_915MHz == RFM_freq)
    {
        spiWrite(RFM_RegFrfMsb, 0xE4u);
        spiWrite(RFM_RegFrfMid, 0x00u);
    }
    else if (RF12_433MHz == RFM_freq)
    {
        spiWrite(RFM_RegFrfMsb, 0x6Cu);
        spiWrite(RFM_RegFrfMid, 0x80u);
    }
    else
    {
        // TODO Breakpoint - should never reach here
    }
    spiWrite(RFM_RefFrfLsb, 0x00u);
}

void
rfm_sendPayload(PayloadTx_t *pSrc, uint8_t group, uint8_t node, uint8_t whitening)
{
    uint8_t *pPayload = (uint8_t *)pSrc;

    /* Whiten the RF if required */
    if (2u == whitening)
    {
        const uint8_t WHITEN = 0x55u;
        for (uint8_t i = 0; i < sizeof(PayloadTx_t); i++)
        {
            *pPayload++ ^= WHITEN;
        }
    }

    uint16_t crc = crc16_update(0xFFFFu, group);
    uint8_t  txState = 0u;
    uint8_t  idxPayload = 0u;
    while (txState < 7u)
    {
        if (1)  // TODO FIFO not full
        {
            uint8_t next;
            switch (txState)
            {
                case 0:
                    next = node & 0x1Fu;
                    txState++;
                    break;
                case 1:
                    next = sizeof(PayloadTx_t);
                    txState++;
                    break;
                case 2:
                    next = pPayload[idxPayload];
                    if (sizeof(PayloadTx_t) == idxPayload) txState++;
                    break;
                case 3:
                    next = (uint8_t)crc;
                    txState++;
                    break;
                case 4:
                    next = (uint8_t)(crc >> 8u);
                    txState++;
                    break;
                case 5:
                case 6:
                    next = 0xAAu;
                    txState++;
                    break;
                default:
                    // TODO Add breakpoint
            }
            if (4 > txState)
            {
                crc = crc16_update(crc, next);
            }
            // TODO Write to RFM
        }
    }
    spiWrite(RFM_RegOpMode, (spiRead(RFM_RegOpMode) & 0xE3u) | MODE_TRANSMITTER);
}
