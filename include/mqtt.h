#ifndef MQTT_H_
#define MQTT_H_

#include "include.h"

template <typename T>
extern void handlePayloadAndQueueUplink(const char *payload);

extern TaskHandle_t MqttTask;

void mqttTask(void *);

void initMqtt();
void handleMqttUplink(const char *topic, const char *payload);
void updateNodeTime();

#endif
