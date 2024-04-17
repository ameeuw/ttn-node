#include "mqtt.h"

TaskHandle_t MqttTask;

PicoMQTT::Server mqtt;

// Tasmota Node registry
std::map<String, tasmotaNode> tasmotaRegistry;

void handleMqttUplink(const char *topic, const char *payload)
{
    String nodeName = mqtt.get_topic_element(topic, 1);
    Serial.printf("topic: '%s'\npayload: %s\nnodeName: %s\n", topic, payload, nodeName);

    if (nodeName == "TRACER")
    {
        handlePayloadAndQueueUplink<tracerStruct>(payload);
    }
    else if (nodeName == "CO2")
    {
        handlePayloadAndQueueUplink<co2Struct>(payload);
    }
    else if (nodeName == "COOLBOX")
    {
        handlePayloadAndQueueUplink<tracerStruct>(payload);
    }
    else
    {
        Serial.println("No parseable data in payload.");
    }
}

void discoveryCallback(const char *topic, const char *payload)
{
    String mac = mqtt.get_topic_element(topic, 2);
    Serial.printf("Received config in topic '%s' (nodeMac = '%s'): %s\n", topic, mac.c_str(), payload);

    if (tasmotaRegistry.find(mac.c_str()) != tasmotaRegistry.end())
    {
        Serial.println("Found node");
    }
    else
    {
        Serial.println("Adding node");
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        String hostname = doc["hn"];
        String ip = doc["ip"];
        String topic = doc["t"];
        tasmotaNode node = {hostname, ip, topic};
        tasmotaRegistry[mac.c_str()] = node;
    }
}

void initMqtt()
{
    // Subscribe to a topic and attach a callback
    mqtt.subscribe("stat/+/STATUS8", handleMqttUplink);
    mqtt.subscribe("tasmota/discovery/+/config", discoveryCallback);
    mqtt.begin();

    xTaskCreatePinnedToCore(
        mqttTask,    /* Task function. */
        "MQTT Task", /* String with name of task. */
        10000,       /* Stack size in words. */
        NULL,        /* Parameter passed as input of the task */
        1,           /* Priority of the task. */
        &MqttTask,   /* Task handle. */
        1            /* Pinned CPU core. */
    );
}

void mqttTask(void *parameter)
{
    const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    while (true)
    {
        mqtt.loop();
        vTaskDelay(xDelay);
    }
}

void updateNodeTime()
{
    for (auto const &pair : tasmotaRegistry)
    {
        String topic = "cmnd/" + pair.second.topic + "/time";
        Serial.printf("Publishing time to %s\n", topic.c_str());
        mqtt.publish(topic, String(now()));
    }
}

void getStatusJson(DynamicJsonDocument &doc)
{
    // volatile UBaseType_t uxArraySize, x;
    // uint32_t ulTotalRunTime, ulStatsAsPercentage;

    // uxArraySize = uxTaskGetNumberOfTasks();
    // TaskStatus_t *pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

    // uxArraySize = uxTaskGetSystemState(pxTaskStatusArray,
    //                                    uxArraySize,
    //                                    &ulTotalRunTime);
    // ulTotalRunTime /= 100UL;

    // for (uint16_t i = 0; i < uxArraySize; i++)
    // {
    //     TaskStatus_t xTaskStatus = pxTaskStatusArray[i];
    //     ulStatsAsPercentage = xTaskStatus.ulRunTimeCounter / ulTotalRunTime;
    //     doc["tasks"][xTaskStatus.pcTaskName]["name"] = xTaskStatus.pcTaskName;
    //     doc["tasks"][xTaskStatus.pcTaskName]["priority"] = xTaskStatus.uxCurrentPriority;
    //     doc["tasks"][xTaskStatus.pcTaskName]["stack"] = xTaskStatus.usStackHighWaterMark;
    //     doc["tasks"][xTaskStatus.pcTaskName]["cpu"] = ulStatsAsPercentage;
    // }
    // vPortFree(pxTaskStatusArray);

    for (auto task : {MqttTask, LmicTask, HandleUplinkMsgTask, HandleDownlinkMsgTask})
    {
        String name = pcTaskGetTaskName(task);
        doc["tasks"][name]["stack"] = uxTaskGetStackHighWaterMark(task);
        doc["tasks"][name]["name"] = name;
        doc["tasks"][name]["priority"] = uxTaskPriorityGet(task);
    }

    // #ifdef USE_RTC
    //     DateTime now = rtc.now();
    //     doc["time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
    // #else
    doc["time"] = now();
    // #endif

    // Tasmota Node registry
    for (auto const &pair : tasmotaRegistry)
    {
        doc["registry"]["tasmota"][pair.first.c_str()]["hostname"] = pair.second.hostname;
        doc["registry"]["tasmota"][pair.first.c_str()]["ip"] = pair.second.ip;
        doc["registry"]["tasmota"][pair.first.c_str()]["topic"] = pair.second.topic;
    }
    // Wifi Client registry
    for (auto const &pair : clientRegistry)
    {
        doc["registry"]["client"][pair.first.c_str()]["hostname"] = pair.second.hostname;
        doc["registry"]["client"][pair.first.c_str()]["ip"] = pair.second.ip;
        doc["registry"]["client"][pair.first.c_str()]["topic"] = pair.second.topic;
    }

    // Lora Stats
    doc["lora"]["up"] = LMIC.seqnoUp;
    doc["lora"]["down"] = LMIC.seqnoDn;
    int16_t snrTenfold = getSnrTenfold();
    int8_t snr = snrTenfold / 10;
    int8_t snrDecimalFraction = snrTenfold % 10;
    int16_t rssi = getRssi(snr);
    doc["lora"]["rssi"] = rssi;
    doc["lora"]["snr"] = float(snr + snrDecimalFraction / 10.0);

    // System stats
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    doc["system"]["features"]["psram"] = chip_info.features & CHIP_FEATURE_EMB_PSRAM;
    doc["system"]["features"]["wifi"] = chip_info.features & CHIP_FEATURE_WIFI_BGN;
    doc["system"]["features"]["bt"] = chip_info.features & CHIP_FEATURE_BT;
    doc["system"]["features"]["ble"] = chip_info.features & CHIP_FEATURE_BLE;

    doc["system"]["cores"] = chip_info.cores;
    doc["system"]["model"] = chip_info.model;

    doc["system"]["program"]["size"] = ESP.getFreeSketchSpace();
    doc["system"]["program"]["used"] = ESP.getSketchSize();

    doc["system"]["fs"]["size"] = LittleFS.totalBytes();
    doc["system"]["fs"]["used"] = LittleFS.usedBytes();

    doc["system"]["flash"]["size"] = ESP.getFlashChipSize();
    doc["system"]["flash"]["speed"] = ESP.getFlashChipSpeed();
    doc["system"]["flash"]["used"] = LittleFS.usedBytes() + ESP.getSketchSize();

    doc["system"]["heap"]["size"] = ESP.getHeapSize();
    doc["system"]["heap"]["free"] = esp_get_free_heap_size();
    doc["system"]["heap"]["minFree"] = esp_get_minimum_free_heap_size();
    doc["system"]["heap"]["minInternal"] = esp_get_free_internal_heap_size();

    doc["system"]["wakeUpReason"] = esp_sleep_get_wakeup_cause();
    doc["system"]["resetReason"] = esp_reset_reason();

    doc["system"]["wifi"]["rssi"] = WiFi.RSSI();
    doc["system"]["wifi"]["mac"] = WiFi.macAddress();
    doc["system"]["wifi"]["channel"] = WiFi.channel();
    if (WiFi.status() == WL_CONNECTED)
        doc["system"]["wifi"]["ip"] = WiFi.localIP().toString();

    doc["system"]["freq"]["cpu"] = getCpuFrequencyMhz();
    doc["system"]["freq"]["xtal"] = getXtalFrequencyMhz();
    doc["system"]["freq"]["abp"] = getApbFrequency();
}

void publishStatusMessage()
{
    String topic = "ludwig/stats";
    DynamicJsonDocument doc(2048);
    getStatusJson(doc);
    String message;
    serializeJson(doc, message);
    mqtt.publish(topic, message);
}