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
    spiPkt_t pkt_w;
    spiPkt_t pkt_r;

    /* REG_OPMODE */
    pkt_w.addr = 0x01u;

    /* REG_IRQFLAGS2: IRQ2_PACKETSENT */
    pkt_r.addr = 0x28u;
    pkt_r.data = 0;

    spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    while (0 == (pkt_r.data & 0x08u))
    {
        timerDelay_us(1000);
        spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    }

    /* REG_OPMODE */
    pkt_r.addr = 0x01u;
    spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    pkt_w.data = (pkt_r.data & 0xE3u) | 0x01u;
    spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
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

    spiPkt_t pkt_w;
    spiPkt_t pkt_r;

    pkt_w.addr = 0x2F;
    pkt_w.data = 0xAAu;
    pkt_r.addr = 0x2F;
    pkt_r.data = 0;

    /* Initialise RFM69 */
    while (0xAA != pkt_r.data)
    {
        spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
        spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    }
    pkt_w.data = 0x55;
    while (0x55 != pkt_r.data)
    {
        spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
        spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    }

    /* Configuration */
    for (unsigned int idxCfg = 0; (255 != config[idxCfg][0]); idxCfg++)
    {
        pkt_w.addr = config[idxCfg][0];
        pkt_w.data = config[idxCfg][1];
        spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
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
    spiPkt_t        pkt_r;
    spiPkt_t        pkt_w;

    /* Wait for "clear" air to transmit in.
     * 1. Enter receive mode
     * 2. Listen until below threshold
     * 3. If over time, then return with failure. Otherwise proceed
     */
    if (0 != pPkt->timeout)
    {
        success = -1;

        pkt_w.addr = 0x01u;
        pkt_r.addr = 0x01u;
        spiReadByte(SERCOM_SPI_DATA, &pkt_r);
        pkt_w.data = (pkt_r.data & 0xE3) | 0x10;

        /* Non-blocking timer was busy */
        timedOut = 0;
        if (-1 == timerDelayNB_us(pPkt->timeout, &setTimedOut))
        {
            return -1;
        }
        while (0 == timedOut)
        {
            /* Wait for READY */
            pkt_r.addr = 0x27u; /* REG_IRQFLAGS1 */
            spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            while(0 == (pkt_r.data & 0x80))
            {
                spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            }

            pkt_w.addr = 0x23u; /* REG_RSSI_CONFIG */
            pkt_w.data = 0x01u; /* RSSI_START */
            spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
            pkt_r.addr = 0x23u;
            spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            while (0 == (pkt_r.data & 0x02u)) /* RSSI_DONE */
            {
                spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            }
            pkt_r.addr = 0x24u; /* REG_RSSI_VALUE */
            spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            if (pkt_r.data > (pPkt->threshold * -2))
            {
                success = 0;
                break;
            }
            /* Restart receiver */
            pkt_r.addr = 0x3Du; /* REG_PACKET_CONFIG2 */
            pkt_w.addr = 0x3Du;
            spiReadByte(SERCOM_SPI_DATA, &pkt_r);
            pkt_w.data = (pkt_r.data & 0xFB) | 0x04u;
            spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
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
    pkt_r.addr = 0x28u;     /* IRQ_INTFLAG2 */
    pkt_w.addr = 0x00u;     /* REG_FIFO */
    while (txState < 5)
    {
        spiReadByte(SERCOM_SPI_DATA, &pkt_r);
        if (0 == (pkt_r.data & 0x80u))
        {
            switch (txState)
            {
                case 0:
                    pkt_w.data = pPkt->node & 0x1F;
                    txState++;
                    break;
                case 1:
                    pkt_w.data = pPkt->n;
                    txState++;
                    break;
                case 2:
                    pkt_w.data = *data++;
                    pPkt->n--;
                    if (0 == pPkt->n)
                    {
                        txState++;
                    }
                    break;
                case 3:
                    pkt_w.data = (uint8_t)crc;
                    txState++;
                    break;
                case 4:
                    pkt_w.data = (uint8_t)(crc >> 8);
                    txState++;
                    break;
            }
            if (txState < 4)
            {
                crc16_update(&crc, pkt_w.data);
            }
            spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
        }
    }
    pkt_w.data = 0xAA;
    while (txState < 17u)
    {
        spiWriteByte(SERCOM_SPI_DATA, &pkt_w);
    }

    pkt_w.addr = 0x11u;
    pkt_w.data = (pPkt->rf_pwr & 0x1F) | 0x80;
    spiWriteByte(SERCOM_SPI_DATA, &pkt_w);

    pkt_w.addr = 0x01u;
    pkt_r.addr = 0x01u;
    spiReadByte(SERCOM_SPI_DATA, &pkt_r);
    pkt_w.data = (pkt_r.data & 0xE3) | 0x0C;
    spiWriteByte(SERCOM_SPI_DATA, &pkt_w);

    rfm_sleep();
    return success;
}
