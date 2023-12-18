#ifndef PARSERS_H_
#define PARSERS_H_

#include "include.h"

extern QueueHandle_t uplinkQueue;

template <typename T>
T parseStruct(DynamicJsonDocument doc, uint16_t counterValue);

template <typename T>
void handlePayloadAndQueueUplink(const char *payload)
{
    T *uplinkPayload = (T *)pvPortMalloc(sizeof(T));
    if (uplinkPayload == NULL)
    {
        Serial.println(F("Failed to allocate heap memory for uplinkPayload."));
    }
    else
    {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        T tempUplinkPayload = parseStruct<T>(doc, 1337);
        memcpy(uplinkPayload, &tempUplinkPayload, sizeof(T));
        Serial.println("Enqueuing telemetry for uplink.");

        linkMessage *ptxuplinkMessage = (linkMessage *)pvPortMalloc(sizeof(linkMessage));
        if (ptxuplinkMessage == NULL)
        {
            Serial.println(F("Failed to allocate heap memory for ptxuplinkMessage."));
        }
        else
        {
            ptxuplinkMessage->fport = T::fport;
            ptxuplinkMessage->length = sizeof(T);
            ptxuplinkMessage->data = (uint8_t *)uplinkPayload;
            xQueueSend(uplinkQueue, &ptxuplinkMessage, (TickType_t)0);
        }
    }
}

#endif
