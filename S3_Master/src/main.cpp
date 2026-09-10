#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#include "air_protocol.h"


// =====================================================
// SX1278 - ESP32-S3 RX
// =====================================================

#define LORA_CS       10
#define LORA_MOSI     11
#define LORA_SCK      12
#define LORA_MISO     13

#define LORA_RST      4
#define LORA_DIO0     5

#define LORA_FREQ     433E6


// =====================================================
// KHỞI TẠO
// =====================================================

void initLoRa()
{
    Serial.println(
        "Khoi tao SX1278 RX..."
    );

    SPI.begin(
        LORA_SCK,
        LORA_MISO,
        LORA_MOSI,
        LORA_CS
    );

    LoRa.setPins(
        LORA_CS,
        LORA_RST,
        LORA_DIO0
    );

    if (!LoRa.begin(LORA_FREQ))
    {
        Serial.println(
            "LOI: Khong tim thay SX1278!"
        );

        while (1)
        {
            delay(1000);
        }
    }


    // PHẢI GIỐNG TX

    LoRa.setSyncWord(0x12);

    LoRa.setSpreadingFactor(7);

    LoRa.setSignalBandwidth(125E3);

    LoRa.setCodingRate4(5);

    LoRa.setPreambleLength(8);

    LoRa.enableCrc();


    Serial.println(
        "SX1278 RX OK!"
    );

    Serial.println(
        "Dang cho packet..."
    );
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1500);

    Serial.println();
    Serial.println("========================");
    Serial.println(" ESP32-S3 LORA GATEWAY");
    Serial.println("========================");

    Serial.print(
        "Expected packet size: "
    );

    Serial.println(
        sizeof(AirPacket)
    );

    initLoRa();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    int packetSize =
        LoRa.parsePacket();

    if (!packetSize)
    {
        return;
    }


    Serial.println();
    Serial.println(
        "========================="
    );

    Serial.println(
        "NHAN DUOC PACKET!"
    );


    // =================================================
    // CHECK SIZE
    // =================================================

    Serial.print("Packet size: ");
    Serial.println(packetSize);

    if (packetSize != sizeof(AirPacket))
    {
        Serial.println(
            "LOI: PACKET SIZE KHONG DUNG!"
        );

        while (LoRa.available())
        {
            LoRa.read();
        }

        return;
    }


    // =================================================
    // READ
    // =================================================

    AirPacket packet;

    uint8_t *buffer =
        (uint8_t *)&packet;

    int index = 0;

    while (
        LoRa.available() &&
        index < sizeof(AirPacket)
    )
    {
        buffer[index++] =
            LoRa.read();
    }


    // =================================================
    // CHECK MAGIC
    // =================================================

    if (packet.magic != PACKET_MAGIC)
    {
        Serial.print(
            "LOI MAGIC: 0x"
        );

        Serial.println(
            packet.magic,
            HEX
        );

        return;
    }


    // =================================================
    // CHECK VERSION
    // =================================================

    if (
        packet.version !=
        PACKET_VERSION
    )
    {
        Serial.println(
            "LOI VERSION!"
        );

        return;
    }


    // =================================================
    // CHECK ADDRESS
    // =================================================

    if (
        packet.dstId !=
        NODE_GATEWAY
    )
    {
        Serial.println(
            "PACKET KHONG GUI CHO GATEWAY!"
        );

        return;
    }


    // =================================================
    // CRC
    // =================================================

    uint16_t crcCalculated =
        calculateCRC16(
            (uint8_t *)&packet,
            offsetof(AirPacket, crc16)
        );


    if (
        crcCalculated !=
        packet.crc16
    )
    {
        Serial.println(
            "LOI CRC!"
        );

        Serial.print(
            "Received: 0x"
        );

        Serial.println(
            packet.crc16,
            HEX
        );

        Serial.print(
            "Calculated: 0x"
        );

        Serial.println(
            crcCalculated,
            HEX
        );

        return;
    }


    // =================================================
    // DECODE DATA
    // =================================================

    float temperature =
        packet.temperature_x100
        / 100.0f;

    float humidity =
        packet.humidity_x100
        / 100.0f;

    float dust =
        packet.dust_x10
        / 10.0f;


    // =================================================
    // PRINT
    // =================================================

    Serial.println(
        ">>> PACKET OK"
    );

    Serial.print("SRC: ");
    Serial.println(packet.srcId);

    Serial.print("DST: ");
    Serial.println(packet.dstId);

    Serial.print("SEQ: ");
    Serial.println(packet.sequence);


    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" C");


    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");


    Serial.print("MQ7 RAW: ");
    Serial.println(packet.mq7_raw);


    Serial.print("MQ135 RAW: ");
    Serial.println(packet.mq135_raw);


    Serial.print("Dust: ");
    Serial.print(dust, 1);
    Serial.println(" ug/m3");


    Serial.print("RSSI: ");
    Serial.print(
        LoRa.packetRssi()
    );
    Serial.println(" dBm");


    Serial.print("SNR: ");
    Serial.print(
        LoRa.packetSnr()
    );
    Serial.println(" dB");


    // =================================================
    // JSON
    // =================================================

    Serial.println();
    Serial.println("JSON:");

    Serial.print("{");

    Serial.print("\"seq\":");
    Serial.print(packet.sequence);

    Serial.print(",");

    Serial.print("\"temperature\":");
    Serial.print(temperature, 2);

    Serial.print(",");

    Serial.print("\"humidity\":");
    Serial.print(humidity, 2);

    Serial.print(",");

    Serial.print("\"mq7\":");
    Serial.print(packet.mq7_raw);

    Serial.print(",");

    Serial.print("\"mq135\":");
    Serial.print(packet.mq135_raw);

    Serial.print(",");

    Serial.print("\"dust\":");
    Serial.print(dust, 1);

    Serial.print(",");

    Serial.print("\"rssi\":");
    Serial.print(
        LoRa.packetRssi()
    );

    Serial.print(",");

    Serial.print("\"snr\":");
    Serial.print(
        LoRa.packetSnr()
    );

    Serial.println("}");
}