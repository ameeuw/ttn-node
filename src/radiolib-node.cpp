#include "radiolib-node.h"

// Forward declarations for utility functions
void printHeader(void);
bool initSerial(unsigned long speed, int16_t timeoutSeconds);
void initDisplay(void);

bool joined = false;

linkMessage downlinkMessage;
QueueHandle_t downlinkQueue = xQueueCreate(10, sizeof(struct linkMessage *));

// RadioLib objects - will be initialized in board-specific files
Module* radio = nullptr;
LoRaWANNode* node = nullptr;

// Helper function to convert ASCII hex string to byte value.
uint8_t ASCII2Hex(const char str[2])
{
    unsigned char ASCII = 0;
    // High Nibble
    if (str[0] >= 'A' && str[0] <= 'F')
        ASCII = str[0] - 'A' + 10;
    else if (str[0] >= 'a' && str[0] <= 'f')
        ASCII = str[0] - 'a' + 10;
    else
        ASCII = str[0] - '0';
    ASCII <<= 4;
    // Low Nibble
    if (str[1] >= 'A' && str[1] <= 'F')
        ASCII |= str[1] - 'A' + 10;
    else if (str[1] >= 'a' && str[1] <= 'f')
        ASCII |= str[1] - 'a' + 10;
    else
        ASCII |= str[1] - '0';
    return ASCII;
}

// LoRaWAN keys and configuration
#ifdef OTAA_ACTIVATION
// Use the existing key arrays from lorawan-keys.h
const uint8_t DEVEUI[8] = {OTAA_DEVEUI};
const uint8_t APPEUI[8] = {OTAA_APPEUI};
const uint8_t APPKEY[16] = {OTAA_APPKEY};

// Copy keys for RadioLib (keys are already in binary format)
void getDevEui(uint8_t *buf)
{
    memcpy(buf, DEVEUI, 8);
}

void getAppEui(uint8_t *buf)
{
    memcpy(buf, APPEUI, 8);
}

void getAppKey(uint8_t *buf)
{
    memcpy(buf, APPKEY, 16);
}
#else
// ABP activation
static const uint32_t DEVADDR = ABP_DEVADDR;
static const uint8_t NWKSKEY[16] = {ABP_NWKSKEY};
static const uint8_t APPSKEY[16] = {ABP_APPSKEY};
#endif

int16_t getSnrTenfold(void)
{
    if (node == nullptr) return 0;
    // RadioLib returns SNR in dB as float, convert to tenths
    return (int16_t)(node->getSNR() * 10.0f);
}

int16_t getRssi(void)
{
    if (node == nullptr) return -999;
    return (int16_t)node->getRSSI();
}

void setupRadioLib(bool adrEnabled = true,
                   uint8_t abpDataRate = DefaultABPDataRate,
                   int8_t abpTxPower = DefaultABPTxPower)
{
    if (node == nullptr) {
        Serial.println(F("Error: LoRaWAN node not initialized"));
        return;
    }

    // Set up LoRaWAN node
    int16_t state = node->begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("LoRaWAN node initialization failed, code "));
        Serial.println(state);
        return;
    }

    // Configure LoRaWAN parameters
    if (activationMode == ActivationMode::OTAA) {
        uint8_t devEui[8], appEui[8], appKey[16];
        getDevEui(devEui);
        getAppEui(appEui);
        getAppKey(appKey);
        
        state = node->beginOTAA(devEui, appEui, appKey);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print(F("OTAA setup failed, code "));
            Serial.println(state);
            return;
        }
    } else {
        // ABP activation
        state = node->beginABP(DEVADDR, NWKSKEY, APPSKEY);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print(F("ABP setup failed, code "));
            Serial.println(state);
            return;
        }
        joined = true; // ABP is considered "joined" immediately
    }

    // Set ADR mode
    node->setADR(adrEnabled);
    
    // Set data rate and power for ABP
    if (activationMode == ActivationMode::ABP) {
        node->setDatarate(abpDataRate);
        node->setTxPower(abpTxPower);
    }

    Serial.println(F("RadioLib LoRaWAN setup complete"));
}

// RadioLib event callback - simplified for now
void onRadioLibEvent(uint16_t eventType, void* eventData)
{
    uint32_t timestamp = millis(); // Use millis() instead of LMIC's os_getTime()

    const String topic = "ludwig/lora/event";
    DynamicJsonDocument doc(1024);
    doc["timestamp"] = timestamp;
    doc["event"] = eventType;
    String message;
    serializeJson(doc, message);
    Serial.println(message);
    mqtt.publish(topic, message);

    switch (eventType) {
        case RADIOLIB_EVENT_JOIN_ACCEPT:
            Serial.println(F("Join accept received"));
            joined = true;
            break;

        case RADIOLIB_EVENT_JOIN_FAILED:
            Serial.println(F("Join failed"));
            joined = false;
            break;

        case RADIOLIB_EVENT_UPLINK_COMPLETE:
            Serial.println(F("Uplink complete"));
            break;

        case RADIOLIB_EVENT_DOWNLINK_RECEIVED:
            Serial.println(F("Downlink received"));
            // Handle downlink data processing here
            break;

        default:
            Serial.print(F("Unknown RadioLib event: "));
            Serial.println(eventType);
            break;
    }
}

int16_t scheduleUplink(uint8_t fPort, uint8_t *data, uint8_t dataLength, bool confirmed)
{
    if (node == nullptr || !joined) {
        return -1; // Not ready
    }

    uint32_t timestamp = millis();
    Serial.print(F("Scheduling uplink on port "));
    Serial.print(fPort);
    Serial.print(F(", length "));
    Serial.println(dataLength);

    int16_t state;
    if (confirmed) {
        state = node->sendReceive(data, dataLength, fPort);
    } else {
        state = node->uplink(data, dataLength, fPort);
    }

    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("Uplink scheduled successfully"));
    } else {
        Serial.print(F("Uplink failed, code "));
        Serial.println(state);
    }

    return state;
}

void processDownlink(uint32_t txCompleteTimestamp, uint8_t fPort, uint8_t *data, uint8_t dataLength)
{
    // This function is called from the RadioLib event handler
    // when a downlink message was received.
    linkMessage *ptxdownlinkMessage = (linkMessage *)pvPortMalloc(sizeof(linkMessage));
    if (ptxdownlinkMessage == NULL) {
        Serial.println(F("Failed to allocate heap memory."));
    } else {
        ptxdownlinkMessage->fport = fPort;
        ptxdownlinkMessage->length = dataLength;
        ptxdownlinkMessage->data = (uint8_t*)pvPortMalloc(dataLength);
        if (ptxdownlinkMessage->data != NULL) {
            memcpy(ptxdownlinkMessage->data, data, dataLength);
            xQueueSend(downlinkQueue, &ptxdownlinkMessage, (TickType_t)0);
        } else {
            vPortFree(ptxdownlinkMessage);
            Serial.println(F("Failed to allocate heap memory for downlink data."));
        }
    }
}

//  █▀▄ ▄▀█ █▀▄ █ █▀█ █   █ █▄▄   ▀█▀ ▄▀█ █▀ █▄▀
//  █▀▄ █▀█ █▄▀ █ █▄█ █▄▄ █ █▄█    █  █▀█ ▄█ █ █

TaskHandle_t RadioLibTask;

void radioLibTask(void *parameter)
{
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS; // 100ms delay
    while (true) {
        // RadioLib handles most operations internally via interrupts
        // This task mainly handles periodic maintenance
        if (node != nullptr) {
            // Process any pending LoRaWAN operations
            node->process();
        }
        vTaskDelay(xDelay);
    }
}

void initRadioLib()
{
    // boardInit(InitType::Hardware) must be called at start of setup() before anything else.
    bool hardwareInitSucceeded = boardInit(InitType::Hardware);

#ifdef USE_DISPLAY
    initDisplay();
#endif

#ifdef USE_SERIAL
    initSerial(MONITOR_SPEED, WAITFOR_SERIAL_S);
#endif

    boardInit(InitType::PostInitSerial);

#if defined(USE_SERIAL) || defined(USE_DISPLAY)
    printHeader();
#endif

    if (!hardwareInitSucceeded) {
#ifdef USE_SERIAL
        serial.println(F("Error: hardware init failed."));
        serial.flush();
#endif
#ifdef USE_DISPLAY
        // Following message shown only if failure was unrelated to I2C.
        display.setCursor(COL_0, FRMCNTRS_ROW);
        display.print(F("HW init failed"));
#endif
        abort();
    }

    setupRadioLib();

    if (activationMode == ActivationMode::OTAA && node != nullptr) {
        Serial.println(F("Starting OTAA join..."));
        int16_t state = node->activate();
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print(F("OTAA activation failed, code "));
            Serial.println(state);
        }
    }

    xTaskCreatePinnedToCore(
        radioLibTask,      /* Task function. */
        "RadioLib Task",   /* String with name of task. */
        30000,            /* Stack size in words. */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task. */
        &RadioLibTask,    /* Task handle. */
        1                 /* Pinned CPU core. */
    );
}
