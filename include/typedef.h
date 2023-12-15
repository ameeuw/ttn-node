#ifndef typedef_h
#define typedef_h

// Payload structs
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
    int32_t t;
    uint16_t counter;
    static constexpr uint8_t fport = 12;
};

struct meterStruct
{
    float batteryVoltage;
    float loadPower;
    float consumptionSum;
    int32_t t;
    uint16_t counter;
    static constexpr uint8_t fport = 13;
};

struct co2Struct
{
    uint16_t co2;
    uint16_t illuminance;
    int32_t t;
    uint16_t counter;
    static constexpr uint8_t fport = 14;
};

struct gpsStruct
{
    double latitude;
    double longitude;
    double altitude;
    double speed;
    int32_t t;
    uint16_t counter;
    static constexpr uint8_t fport = 15;
};

struct coolboxStruct
{
    float inflowTemperature;
    float outflowTemperature;
    uint16_t flowCounter;
    uint16_t tec1Current;
    uint16_t tec2Current;
    float waterTemperature;
    float airTemperature;
    float humidity;
    int32_t t;
    uint16_t counter;
    static constexpr uint8_t fport = 16;
};

// Message queue stucts

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

#endif
