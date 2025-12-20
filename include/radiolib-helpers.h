#ifndef RADIOLIB_HELPERS_H_
#define RADIOLIB_HELPERS_H_

#include "include.h"

#ifdef USE_RTC
extern RTC_DS1307 rtc;
#endif // USE_RTC

extern QueueHandle_t downlinkQueue;

extern TaskHandle_t HandleDownlinkMsgTask;
extern TaskHandle_t HandleUplinkMsgTask;

extern CircularBuffer lastUplinks;
extern CircularBuffer lastDownlinks;

void initHelperTasks(void);
void handleUplinkMsgTask(void *);
void handleDownlinkMsgTask(void *parameter);

extern PicoMQTT::Server mqtt;

#endif // RADIOLIB_HELPERS_H_
