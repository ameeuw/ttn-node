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
    mqtt.subscribe("tele/+/SENSOR", handleMqttUplink);
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

void publishStatusMessage()
{
    updateClientRegistry();
    String topic = "ludwig/stats";
    DynamicJsonDocument doc(1024);
    doc["tasks"]["MqttTask"] = uxTaskGetStackHighWaterMark(MqttTask);
    doc["tasks"]["LmicTask"] = uxTaskGetStackHighWaterMark(LmicTask);
    doc["tasks"]["HandleUplinkMsgTask"] = uxTaskGetStackHighWaterMark(HandleUplinkMsgTask);
    doc["tasks"]["HandleDownlinkMsgTask"] = uxTaskGetStackHighWaterMark(HandleDownlinkMsgTask);
#ifdef USE_RTC
    DateTime now = rtc.now();
    doc["Time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
#else
    doc["Time"] = now();
#endif

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

    String message;
    serializeJson(doc, message);
    mqtt.publish(topic, message);
}