#ifndef typedef_h
#define typedef_h

struct tracerStruct
{
    float batteryTemperature;
    float batterySoc;
    float batteryVoltage;
    float batteryCurrent;
    float pvVoltage;
    float pvCurrent;
    float pvPower;
    float loadVoltage;
    float loadCurrent;
    float loadPower;
    float consumptionDay;
    float consumptionSum;
    float productionSum;
    float batteryMaxVoltage;
    uint16_t counter;
    uint32_t t;
};

struct meterStruct
{
    float batteryVoltage;
    float loadPower;
    float consumptionSum;
    uint32_t t;
    uint16_t counter;
};

struct co2Struct
{
    uint16_t co2;
    uint16_t illuminance;
    uint32_t t;
    uint16_t counter;
};

struct gpsStruct
{
    double latitude;
    double longitude;
    double altitude;
    uint32_t t;
    uint16_t counter;
};

typedef struct qMessage
{
    uint8_t id;
    uint8_t length;
    char payload[50];
} qMessage;

typedef struct linkMessage
{
    uint8_t fport;
    uint8_t *data;
    uint8_t length;
} linkMessage;

typedef struct mqttMessage
{
    String topic;
    String payload;
} mqttMessage;

#include "../keyfiles/lorawan-keys.h"

#endif
