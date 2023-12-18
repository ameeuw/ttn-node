#ifndef LMIC_HELPERS_H_
#define LMIC_HELPERS_H_

#include "include.h"

extern QueueHandle_t downlinkQueue;

extern TaskHandle_t HandleDownlinkMsgTask;
extern TaskHandle_t HandleUplinkMsgTask;

void initHelperTasks(void);
void handleUplinkMsgTask(void *);
void handleDownlinkMsgTask(void *parameter);

extern PicoMQTT::Server mqtt;

#endif // LMIC_HELPERS_H_
