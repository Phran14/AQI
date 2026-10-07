#include <Arduino.h>
#include <DHT.h>

// =====================================================
// PIN ESP32 38 PIN
// =====================================================

#define MQ7_PIN        32
#define MQ135_PIN      34

#define DUST_LED_PIN   21
#define DUST_VO_PIN    35

#define DHT22_PIN      27
#define DHT_TYPE       DHT22

DHT dht(DHT22_PIN, DHT_TYPE);


// =====================================================
// CAU CHIA AP
// =====================================================

// MQ7:
// AO --- 22k --- GPIO32 --- 22k --- GND
const float MQ7_R1 = 22000.0f;
const float MQ7_R2 = 22000.0f;

// MQ135:
// AO --- 10k --- GPIO34 --- 18k --- GND
const float MQ135_R1 = 10000.0f;
const float MQ135_R2 = 18000.0f;

// GP2Y:
// Vo --- 10k --- GPIO35 --- 18k --- GND
const float DUST_R1 = 10000.0f;
const float DUST_R2 = 18000.0f;


// =====================================================
// ADC
// =====================================================

const int ADC_SAMPLE_COUNT = 32;


// =====================================================
// GP2Y
// =====================================================

// So pulse dung de lay trung binh
const int DUST_SAMPLE_COUNT = 20;


// =====================================================
// HAM KHOI PHUC DIEN AP TRUOC CAU CHIA
// =====================================================

float restoreVoltage(float vadc, float R1, float R2)
{
    return vadc * (R1 + R2) / R2;
}


// =====================================================
// DOC ADC ON DINH
//
// Lay nhieu sample -> bo sample max/min -> lay trung binh
// =====================================================

float readStableADCVoltage(int pin)
{
    uint32_t sum = 0;

    uint32_t minValue = 100000;
    uint32_t maxValue = 0;

    for (int i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        uint32_t mv = analogReadMilliVolts(pin);

        sum += mv;

        if (mv < minValue)
            minValue = mv;

        if (mv > maxValue)
            maxValue = mv;

        delay(2);
    }

    // Bo 1 mau cao nhat + 1 mau thap nhat
    float averageMv =
        (sum - minValue - maxValue)
        /
        (float)(ADC_SAMPLE_COUNT - 2);

    return averageMv / 1000.0f;
}


// =====================================================
// DOC MQ7
// =====================================================

float readMQ7Voltage()
{
    float vadc =
        readStableADCVoltage(MQ7_PIN);

    float vao =
        restoreVoltage(
            vadc,
            MQ7_R1,
            MQ7_R2
        );

    return vao;
}


// =====================================================
// DOC MQ135
// =====================================================

float readMQ135Voltage()
{
    float vadc =
        readStableADCVoltage(MQ135_PIN);

    float vao =
        restoreVoltage(
            vadc,
            MQ135_R1,
            MQ135_R2
        );

    return vao;
}


// =====================================================
// DOC 1 PULSE GP2Y1010AU0F
// =====================================================

float readDustPulse()
{
    // -------------------------------
    // Bat LED
    // Active LOW
    // -------------------------------

    digitalWrite(DUST_LED_PIN, LOW);

    // Cho 280 us roi lay mau
    delayMicroseconds(280);

    uint32_t mv =
        analogReadMilliVolts(DUST_VO_PIN);

    // Du pulse 320 us
    delayMicroseconds(40);

    // Tat LED
    digitalWrite(DUST_LED_PIN, HIGH);

    // Hoan thanh chu ky ~10 ms
    delayMicroseconds(9680);

    float vadc =
        mv / 1000.0f;

    float vo =
        restoreVoltage(
            vadc,
            DUST_R1,
            DUST_R2
        );

    return vo;
}


// =====================================================
// DOC GP2Y ON DINH
//
// Doc 20 pulse
// Bo max/min
// Sau do lay trung binh
// =====================================================

float readDustVoltage()
{
    float sum = 0.0f;

    float minValue = 100.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < DUST_SAMPLE_COUNT; i++)
    {
        float vo = readDustPulse();

        sum += vo;

        if (vo < minValue)
            minValue = vo;

        if (vo > maxValue)
            maxValue = vo;
    }

    return
        (sum - minValue - maxValue)
        /
        (DUST_SAMPLE_COUNT - 2);
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    // ------------------------------
    // ADC
    // ------------------------------

    analogReadResolution(12);

    analogSetPinAttenuation(
        MQ7_PIN,
        ADC_11db
    );

    analogSetPinAttenuation(
        MQ135_PIN,
        ADC_11db
    );

    analogSetPinAttenuation(
        DUST_VO_PIN,
        ADC_11db
    );

    // ------------------------------
    // GP2Y LED
    // ------------------------------

    pinMode(
        DUST_LED_PIN,
        OUTPUT
    );

    // LED OFF
    digitalWrite(
        DUST_LED_PIN,
        HIGH
    );

    // ------------------------------
    // DHT22
    // ------------------------------

    dht.begin();

    delay(2000);

    Serial.println();
    Serial.println(
        "========== AIR SENSOR TEST =========="
    );

    Serial.println(
        "Time(s),MQ7(V),MQ135(V),DustVo(V),Temp(C),Humidity(%)"
    );
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
    // =================================
    // MQ7
    // =================================

    float mq7Voltage =
        readMQ7Voltage();


    // =================================
    // MQ135
    // =================================

    float mq135Voltage =
        readMQ135Voltage();


    // =================================
    // GP2Y
    // =================================

    float dustVoltage =
        readDustVoltage();


    // =================================
    // DHT22
    // =================================

    float temperature =
        dht.readTemperature();

    float humidity =
        dht.readHumidity();


    // =================================
    // SERIAL
    // =================================

    Serial.print(
        millis() / 1000.0f,
        1
    );

    Serial.print(',');

    Serial.print(
        mq7Voltage,
        3
    );

    Serial.print(',');

    Serial.print(
        mq135Voltage,
        3
    );

    Serial.print(',');

    Serial.print(
        dustVoltage,
        3
    );

    Serial.print(',');

    if (isnan(temperature))
        Serial.print("NaN");
    else
        Serial.print(
            temperature,
            1
        );

    Serial.print(',');

    if (isnan(humidity))
        Serial.println("NaN");
    else
        Serial.println(
            humidity,
            1
        );

    // DHT22 khong nen doc qua nhanh
    delay(2000);
}