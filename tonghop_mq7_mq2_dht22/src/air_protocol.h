#ifndef AIR_PROTOCOL_H
#define AIR_PROTOCOL_H

#include <Arduino.h>
#include <stddef.h>

#define PACKET_MAGIC    0xA517
#define PACKET_VERSION  1

#define NODE_SENSOR     0x01
#define NODE_GATEWAY    0x10
#define NODE_BROADCAST  0xFF

#define FLAG_DHT_OK     (1 << 0)
#define FLAG_DUST_OK    (1 << 1)
#define FLAG_MQ7_OK     (1 << 2)
#define FLAG_MQ135_OK   (1 << 3)

struct __attribute__((packed)) AirPacket
{
    uint16_t magic;

    uint8_t version;

    uint8_t srcId;
    uint8_t dstId;

    uint8_t flags;

    uint32_t sequence;

    int16_t temperature_x100;
    uint16_t humidity_x100;

    uint16_t mq7_raw;
    uint16_t mq135_raw;

    uint16_t dust_x10;

    uint16_t crc16;
};

static_assert(sizeof(AirPacket) == 22,
              "AirPacket size error");

// CRC16-CCITT
inline uint16_t calculateCRC16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }

    return crc;
}

#endif