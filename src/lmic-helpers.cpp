#include "lmic-helpers.h"

// LoRaWAN queues
linkMessage uplinklinkMessage;
QueueHandle_t uplinkQueue = xQueueCreate(10, sizeof(struct linkMessage *));
TaskHandle_t HandleDownlinkMsgTask;
TaskHandle_t HandleUplinkMsgTask;

void initHelperTasks(void)
{
    xTaskCreatePinnedToCore(
        handleUplinkMsgTask,  /* Task function. */
        "Handle Uplink Task", /* String with name of task. */
        10000,                /* Stack size in words. */
        NULL,                 /* Parameter passed as input of the task */
        3,                    /* Priority of the task. */
        &HandleUplinkMsgTask, /* Task handle. */
        1                     /* Pinned CPU core. */
    );

    xTaskCreatePinnedToCore(
        handleDownlinkMsgTask,  /* Task function. */
        "Handle Downlink Task", /* String with name of task. */
        10000,                  /* Stack size in words. */
        NULL,                   /* Parameter passed as input of the task */
        2,                      /* Priority of the task. */
        &HandleDownlinkMsgTask, /* Task handle. */
        1                       /* Pinned CPU core. */
    );
}

String publishLinkMessage(const char *topic, const linkMessage *pMessage)
{
    DynamicJsonDocument doc(1024);
    doc["fport"] = pMessage->fport;
    doc["length"] = pMessage->length;
    JsonArray data = doc.createNestedArray("data");
    for (uint8_t i = 0; i < (uint8_t)pMessage->length; i++)
    {
        data.add(pMessage->data[i]);
    }
    String message;
    serializeJson(doc, message);
    mqtt.publish(topic, message);
    return message;
}

void handleUplinkMsgTask(void *parameter)
{
    uint8_t count = 0;
    const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
    struct linkMessage *prxuplinkMessage;

    while (true)
    {
        UBaseType_t uplinkMessagesWaiting = uxQueueMessagesWaiting(uplinkQueue);
        Serial.printf("%d uplink(s) waiting...\n", uplinkMessagesWaiting);

        if (joined)
        {
            Serial.printf("Working uplinkQueue.\n");

            // Pend on new message in queue and forward it to the corresponding handler
            if (xQueueReceive(uplinkQueue, &(prxuplinkMessage), portMAX_DELAY))
            {
                Serial.println("Got pending uplink message");
                Serial.printf("fport: %d; length: %d\n", prxuplinkMessage->fport, prxuplinkMessage->length);

                if (LMIC.devaddr != 0)
                {
                    if (LMIC.opmode & OP_TXRXPEND)
                    {
                        Serial.println("Uplink not scheduled because TxRx pending");
                    }
                    else
                    {
                        scheduleUplink(prxuplinkMessage->fport, prxuplinkMessage->data, prxuplinkMessage->length, false);
                        lastUplinks.add(*prxuplinkMessage);
                        String topic = "ludwig/lora/uplink";
                        String message = publishLinkMessage(topic.c_str(), prxuplinkMessage);
                        Serial.printf("Published message in topic '%s': %s\n", topic.c_str(), message.c_str());
                    }
                    count = 0;
                }

                vPortFree(prxuplinkMessage);
                vPortFree(prxuplinkMessage->data);
            }
        }
        else
        {
            Serial.println("Not joined yet.");
        }

        vTaskDelay(xDelay);
    }
}

void handleDownlinkMsgTask(void *parameter)
{
    struct linkMessage *prxdownlinkMessage;

    const uint8_t cmdPort = 100;
    const uint8_t resetCmd = 0xC0;
    const uint8_t setTimeCmd = 0xC1;

    while (true)
    {
        // Pend on new message in queue and forward it to the corresponding handler
        if (xQueueReceive(downlinkQueue, &(prxdownlinkMessage), portMAX_DELAY))
        {
            Serial.println("Received downlinkQueue");
            Serial.printf("fport: %d; length: %d\n", prxdownlinkMessage->fport, prxdownlinkMessage->length);
            Serial.print("payload: ");
            for (uint8_t i = 0; i < (uint8_t)prxdownlinkMessage->length; i++)
            {
                Serial.print(prxdownlinkMessage->data[i]);
            }

            String topic = "ludwig/lora/downlink";
            String message = publishLinkMessage(topic.c_str(), prxdownlinkMessage);

            Serial.printf("Published message in topic '%s': %s\n", topic.c_str(), message.c_str());

            if (prxdownlinkMessage->fport == cmdPort && prxdownlinkMessage->length == 1 && prxdownlinkMessage->data[0] == resetCmd)
            {
                serial.println("Reset cmd received.");
            }
            else if (prxdownlinkMessage->fport == cmdPort && prxdownlinkMessage->data[0] == setTimeCmd && prxdownlinkMessage->length == 5)
            {
                uint32_t unixtime = prxdownlinkMessage->data[1] | (uint32_t)prxdownlinkMessage->data[2] << 8 | (uint32_t)prxdownlinkMessage->data[3] << 16 | (uint32_t)prxdownlinkMessage->data[4] << 24;
                Serial.printf("Setting RTC to unix time: %d\n", unixtime);
#ifdef USE_RTC
                rtc.adjust(unixtime);
#else
                setTime(unixtime);
#endif
            }
            lastDownlinks.add(*prxdownlinkMessage);
            vPortFree(prxdownlinkMessage);
        }
    }
}
