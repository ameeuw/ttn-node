#ifndef MQTT_H_
#define MQTT_H_

#include "include.h"

template <typename T>
extern void handlePayloadAndQueueUplink(const char *payload);
void getStatusJson(DynamicJsonDocument &doc);

extern TaskHandle_t MqttTask;
extern TaskHandle_t LmicTask;
void publishStatusMessage();

void mqttTask(void *);
extern std::map<String, tasmotaNode> tasmotaRegistry;

void initMqtt();
void handleMqttUplink(const char *topic, const char *payload);
void updateNodeTime();

#endif
