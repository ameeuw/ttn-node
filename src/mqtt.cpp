#include "mqtt.h"

TaskHandle_t MqttTask;

PicoMQTT::Server mqtt;

// Node registry
std::map<String, tasmotaNode> nodeRegistry;

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

    if (nodeRegistry.find(mac.c_str()) != nodeRegistry.end())
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
        nodeRegistry[mac.c_str()] = node;
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
    for (auto const &pair : nodeRegistry)
    {
        String topic = "cmnd/" + pair.second.topic + "/time";
        Serial.printf("Publishing time to %s\n", topic.c_str());
        mqtt.publish(topic, String(now()));
    }
}
