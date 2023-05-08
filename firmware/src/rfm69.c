#include <stdint.h>

#include "emon32_samd.h"

static volatile unsigned int timedOut = 0;

/* Call back function for the non-blocking timer to set the timed out flag */
void
setTimedOut()
{
    timedOut = 1u;
}

/* Adapted from AVR GCC libc:
 * https://www.nongnu.org/avr-libc/user-manual/group__util__crc.html#ga95371c87f25b0a2497d9cba13190847f
 */
static void
crc16_update(uint16_t *crc, const uint8_t d)
{
    *crc ^= d;
    for (unsigned int i = 0; i < 8; i++)
    {
        *crc =   (*crc & 0x1u)
               ? (*crc >> 1) ^ 0xA001u
               : (*crc >> 1);
    }
}

static void
rfm_sleep()
{
    uint8_t tempRecv;

    /* REG_IRQFLAGS2: IRQ2_PACKETSENT */
    while (0 == (spiReadByte(SERCOM_SPI_DATA, 0x28u) & 0x8u))
    {
        timerDelay_us(1000);
    }

    /* REG_OPMODE */
    tempRecv = spiReadByte(SERCOM_SPI_DATA, 0x1u);
    tempRecv = (tempRecv & 0xE3u) | 0x1u;
    spiWriteByte(SERCOM_SPI_DATA, 0x1u, tempRecv);
}

void
rfm_init(RFM_Freq_t freq)
{
    /* Configuration parameters */
    const uint8_t config[][2] =
    {
        {0x01, 0x04}, /* OPMODE: Sequencer, standby, listen off */
        {0x02, 0x00}, /* DataModul: Packet, FSK, no shaping */
        {0x03, 0x02}, /* BitRate MSB: ~49.23 Kbps */
        {0x04, 0x8A}, /* BitRate LSB */
        {0x05, 0x05}, /* FdevMsb: ~90 kHz */
        {0x06, 0xC3}, /* FdevLsb */
        {0x07, (  RF12_868MHz == freq)
                ? 0xD9
                : (   (RF12_915MHz == freq)
                    ? 0xE4
                    : 0x6C)}, /* FrfMsb */
        {0x08, 0x00}, /* FrfMid */
        {0x09, 0x00}, /* FrfLsb */
        {0x11, 0x99}, /* PaLevel */
        {0x1E, 0x2C},
        {0x25, 0x80}, /* DioMapping1 */
        {0x26, 0x03}, /* DioMapping2 */
        {0x28, 0x00}, /* IrqFlags: FIFO overrun */
        {0x2E, 0x88}, /* SyncConfig : On, FIFO fill, 2 bytes, Tol */
        {0x2F, 0x2D}, /* SyncValue1 */
        {0x37, 0x00}, /* PktConfig: fixed, !DC free, !CRC, !CRCClear */
        {0xFF, 0}
    };

    /* Initialise RFM69 */
    while (0xAA != spiReadByte(SERCOM_SPI_DATA, 0x2Fu))
    {
        spiWriteByte(SERCOM_SPI_DATA, 0x2Fu, 0xAAu);
    }
    while (0x55u != spiReadByte(SERCOM_SPI_DATA, 0x2Fu))
    {
        spiWriteByte(SERCOM_SPI_DATA, 0x2Fu, 0x55u);
    }

    /* Configuration */
    for (unsigned int idxCfg = 0; (0xFF != config[idxCfg][0]); idxCfg++)
    {
        spiWriteByte(SERCOM_SPI_DATA, config[idxCfg][0], config[idxCfg][1]);
    }

    rfm_sleep();
}

int
rfm_send(RFMPkt_t *pPkt)
{
    unsigned int    txState = 0;
    int             success = 0;
    uint16_t        crc     = ~0;
    uint8_t         *data   = (uint8_t *)pPkt->data;
    uint8_t         tempRecv;
    uint8_t         writeByte;

    /* Wait for "clear" air to transmit in.
     * 1. Enter receive mode
     * 2. Listen until below threshold
     * 3. If over time, then return with failure. Otherwise proceed
     */
    if (0 != pPkt->timeout)
    {
        success = -1;

        tempRecv = spiReadByte(SERCOM_SPI_DATA, 0x1u);
        tempRecv = (tempRecv & 0xE3) | 0x10;

        /* Non-blocking timer was busy */
        timedOut = 0;
        if (-1 == timerDelayNB_us(pPkt->timeout, &setTimedOut))
        {
            return -1;
        }
        while (0 == timedOut)
        {
            /* Wait for READY */
            while(0 == (spiReadByte(SERCOM_SPI_DATA, 0x27) & 0x80));

            /* REG_RSSI_CONFIG: RSSI_START */
            spiWriteByte(SERCOM_SPI_DATA, 0x23u, 0x1u);
            /* RSSI_DONE */
            while (0 == (spiReadByte(SERCOM_SPI_DATA, 0x23u) & 0x02u));

            /* REG_RSSI_VALUE */
            if (spiReadByte(SERCOM_SPI_DATA, 0x24u) > (pPkt->threshold * -2))
            {
                success = 0;
                break;
            }
            /* Restart receiver */
            /* REG_PACKET_CONFIG2 */
            tempRecv = spiReadByte(SERCOM_SPI_DATA, 0x3Du);
            tempRecv = (tempRecv & 0xFB) | 0x4u;
            spiWriteByte(SERCOM_SPI_DATA, 0x3Du, tempRecv);
        }
        timerDisable();
    }


    /* 1. Check for FIFO full each loop, then push into FIFO:
     *  - Node information (CTL, DST, ACK, ID
     *  - Number of payload bytes
     *  - Data bytes
     *  - CRC16
     * 1.1 If <12 bytes in the FIFO, fill with dummy bytes
     * 2. Send at specified RF power
     * 3. Enter sleep mode
     */
    crc16_update(&crc, pPkt->grp);
    while (txState < 5)
    {
        if (0 == (spiReadByte(SERCOM_SPI_DATA, 0x28u) & 0x80u))
        {
            switch (txState)
            {
                case 0:
                    writeByte = pPkt->node & 0x1F;
                    txState++;
                    break;
                case 1:
                    writeByte = pPkt->n;
                    txState++;
                    break;
                case 2:
                    writeByte = *data++;
                    pPkt->n--;
                    if (0 == pPkt->n)
                    {
                        txState++;
                    }
                    break;
                case 3:
                    writeByte = (uint8_t)crc;
                    txState++;
                    break;
                case 4:
                    writeByte = (uint8_t)(crc >> 8);
                    txState++;
                    break;
            }
            if (txState < 4)
            {
                crc16_update(&crc, writeByte);
            }
            spiWriteByte(SERCOM_SPI_DATA, 0x0u, writeByte);
        }
    }

    /* Pad FIFO out to minimum 17 bytes */
    while (txState < 17u)
    {
        spiWriteByte(SERCOM_SPI_DATA, 0x0u, 0xAAu);
        txState++;
    }

    writeByte = (pPkt->rf_pwr & 0x1F) | 0x80;
    spiWriteByte(SERCOM_SPI_DATA, 0x11u, writeByte);

    tempRecv = spiReadByte(SERCOM_SPI_DATA, 0x1u);
    tempRecv = (tempRecv & 0xE3) | 0xC;
    spiWriteByte(SERCOM_SPI_DATA, 0x1u, tempRecv);

    rfm_sleep();
    return success;
}
