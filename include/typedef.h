#ifndef typedef_h
#define typedef_h

// Payload structs

struct telemetry
{
    float temp;
    float hum;
    float voltage;
    float current;
    float power;
    uint16_t counter;
    uint32_t t;
    static constexpr uint8_t fport = 11;
};

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

typedef struct tasmotaNode
{
    String hostname;
    String ip;
    String topic;
} tasmotaNode;

class CircularBuffer
{
private:
    static const int capacity = 10;
    linkMessage buffer[capacity];
    int head;
    int tail;
    bool full;

public:
    CircularBuffer() : head(0), tail(0), full(false) {}

    void add(const linkMessage &message)
    {
        buffer[tail].fport = message.fport;
        buffer[tail].length = message.length;
        vPortFree(buffer[tail].data);                                                  // free old message data region
        buffer[tail].data = (uint8_t *)pvPortMalloc(message.length * sizeof(uint8_t)); // allocate new message data region
        memcpy(buffer[tail].data, message.data, message.length * sizeof(uint8_t));     // copy new message data to region

        tail = (tail + 1) % capacity;

        if (full)
        {
            head = (head + 1) % capacity;
        }

        full = full || tail == head;
    }

    void print()
    {
        int count = size();
        Serial.printf("count: %d, head: %d, tail: %d\n", count, head, tail);
        for (int i = 0; i < count; i++)
        {
            int index = (head + i) % capacity;
            Serial.printf("index: %d, fPort: %d, length: %d, data: ", index, buffer[index].fport, buffer[index].length);
            for (uint8_t i = 0; i < buffer[index].length; i++)
            {
                Serial.printf("%d ", buffer[index].data[i]);
            }
            Serial.println();
        }
    }

    void getLoraStatusJson(DynamicJsonDocument &doc)
    {
        int count = size();
        JsonArray last = doc["lora"]["uplink"].createNestedArray("last");
        for (int i = 0; i < count; i++)
        {
            int index = (head + i) % capacity;

            DynamicJsonDocument msg(1024);
            JsonArray bytes = msg.createNestedArray("bytes");
            msg["fPort"] = buffer[index].fport;
            msg["length"] = buffer[index].length;
            for (uint8_t i = 0; i < buffer[index].length; i++)
            {
                bytes.add(buffer[index].data[i]);
            }
            last.add(msg);
        }
    }

    int size() const
    {
        if (full)
        {
            return capacity;
        }
        if (tail >= head)
        {
            return tail - head;
        }
        return capacity + tail - head;
    }
};

#endif
