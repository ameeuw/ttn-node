#ifndef typedef_h
#define typedef_h

struct tracer
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

struct meter
{
    float batteryVoltage;
    float loadPower;
    float consumptionSum;
    uint32_t t;
    uint16_t counter;
};

typedef struct qMessage
{
    uint8_t id;
    uint8_t length;
    char payload[50];
} qMessage;

#include "../keyfiles/lorawan-keys.h"

#endif
