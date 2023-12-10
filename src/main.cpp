#include <include.h>
#include "RTClib.h"

RTC_DS1307 rtc;

PicoMQTT::Server mqtt;

// Task definitions
void lmicTask(void *);
void mqttTask(void *);
void handleUplinkMsgTask(void *);
void handleDownlinkMsgTask(void *parameter);
void printStatusMsgTask(void *parameter);

// LoRaWAN queues
extern QueueHandle_t downlinkQueue;
linkMessage uplinklinkMessage;
QueueHandle_t uplinkQueue = xQueueCreate(10, sizeof(struct linkMessage *));

TaskHandle_t LmicTask;
TaskHandle_t MqttTask;
TaskHandle_t HandleUplinkMsgTask;
TaskHandle_t HandleDownlinkMsgTask;

void initTasks(void)
{
  xTaskCreatePinnedToCore(
      lmicTask,    /* Task function. */
      "LMIC Task", /* String with name of task. */
      30000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      1,           /* Priority of the task. */
      &LmicTask,   /* Task handle. */
      1            /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      mqttTask,    /* Task function. */
      "MQTT Task", /* String with name of task. */
      10000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      1,           /* Priority of the task. */
      &MqttTask,   /* Task handle. */
      1            /* Pinned CPU core. */
  );

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

  xTaskCreatePinnedToCore(
      printStatusMsgTask,  /* Task function. */
      "Print Status Task", /* String with name of task. */
      10000,               /* Stack size in words. */
      NULL,                /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,                /* Task handle. */
      1                    /* Pinned CPU core. */
  );
}

void forwardMqttToQueue(const char *topic, const char *payload);

void initMqtt()
{
  // Subscribe to a topic and attach a callback
  mqtt.subscribe("tele/+/SENSOR", forwardMqttToQueue);
  mqtt.begin();
}

void forwardMqttToQueue(const char *topic, const char *payload)
{
  Serial.printf("Received message in topic '%s': %s\n", topic, payload);
  Serial.print("Allocating memory. Free heap: ");
  Serial.println(xPortGetFreeHeapSize());

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  const JsonObject status = doc["StatusSNS"];
  if (status.containsKey("CO2"))
  {
    co2 tempCo2Payload = parseCo2Struct(status, 1337);

    co2 *co2payload = (co2 *)pvPortMalloc(sizeof(co2));
    if (co2payload == NULL)
    {
      Serial.println(F("Failed to allocate heap memory for co2payload."));
    }
    else
    {
      memcpy(co2payload, &tempCo2Payload, sizeof(co2));

      Serial.print("Enqueuing co2 telemetry for uplink.");
      Serial.println("T: " + String(co2payload->t));
      Serial.println("CO2: " + String(co2payload->co2));
      Serial.println("Illuminance: " + String(co2payload->illuminance));
      Serial.println("Counter: " + String(co2payload->counter));

      linkMessage *ptxuplinkMessage = (linkMessage *)pvPortMalloc(sizeof(linkMessage));
      if (ptxuplinkMessage == NULL)
      {
        Serial.println(F("Failed to allocate heap memory for ptxuplinkMessage."));
      }
      else
      {
        ptxuplinkMessage->fport = 14;
        ptxuplinkMessage->length = sizeof(co2);
        ptxuplinkMessage->data = (uint8_t *)co2payload;
        xQueueSend(uplinkQueue, &ptxuplinkMessage, (TickType_t)0);
      }
    }
  }
  else
  {
    Serial.println("No parseable data in payload.");
    String nodeName = mqtt.get_topic_element(topic, 1);
    Serial.printf("topic: '%s'; payload: %s; nodeName: %s\n", topic, payload, nodeName);
  }
}

void setup()
{
#if defined(DEV2)
  WiFi.softAP("TTGO_v2_0002");
#elif defined(DEV3)
  WiFi.softAP("LOPY_0001");
#elif defined(DEV4)
  WiFi.softAP("LOPY_0002");
  if (!MDNS.begin("ludwig"))
    Serial.println("Error setting up MDNS responder!");
#else
  WiFi.softAP("MyESP32AP");
#endif
  serial.begin(115200);

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    // while (1)
    //   delay(10);
  }
  else
  {
    Serial.println("RTC found");
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  else
  {
    Serial.println("RTC is running");
  }

  delay(1000);
  DateTime now = rtc.now();
  Serial.println(String("Time:\t") + now.timestamp(DateTime::TIMESTAMP_FULL));
  // put your setup code here, to run once:
  setupLmic();
  initMqtt();
  initTasks();
}

void loop()
{
}

void lmicTask(void *parameter)
{
  const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
  while (true)
  {
    os_runloop_once();
    vTaskDelay(xDelay);
  }
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

      String topic = "ludwig/downlink";
      DynamicJsonDocument doc(1024);
      doc["fport"] = prxdownlinkMessage->fport;
      doc["length"] = prxdownlinkMessage->length;
      JsonArray data = doc.createNestedArray("data");
      for (uint8_t i = 0; i < (uint8_t)prxdownlinkMessage->length; i++)
      {
        data.add(prxdownlinkMessage->data[i]);
      }
      String message;
      serializeJson(doc, message);
      Serial.println(message);
      mqtt.publish(topic, message);

      Serial.printf("Publishing message in topic '%s': %s\n", topic.c_str(), message.c_str());

      if (prxdownlinkMessage->fport == cmdPort && prxdownlinkMessage->length == 1 && prxdownlinkMessage->data[0] == resetCmd)
      {
        serial.println("Reset cmd received.");
      }
      else if (prxdownlinkMessage->fport == cmdPort && prxdownlinkMessage->data[0] == setTimeCmd && prxdownlinkMessage->length == 5)
      {
        uint32_t unixtime = prxdownlinkMessage->data[1] | (uint32_t)prxdownlinkMessage->data[2] << 8 | (uint32_t)prxdownlinkMessage->data[3] << 16 | (uint32_t)prxdownlinkMessage->data[4] << 24;
        Serial.printf("Setting RTC to unix time: %d\n", unixtime);
        rtc.adjust(unixtime);
      }
      vPortFree(prxdownlinkMessage);
    }
  }
}

void printStatusMsgTask(void *parameter)
{
  const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;

  while (true)
  {
    DateTime now = rtc.now();
    String topic = "ludwig/stats";
    DynamicJsonDocument doc(1024);
    doc["MqttTask"] = uxTaskGetStackHighWaterMark(MqttTask);
    doc["LmicTask"] = uxTaskGetStackHighWaterMark(LmicTask);
    doc["HandleUplinkMsgTask"] = uxTaskGetStackHighWaterMark(HandleUplinkMsgTask);
    doc["HandleDownlinkMsgTask"] = uxTaskGetStackHighWaterMark(HandleDownlinkMsgTask);
    doc["Time"] = now.timestamp(DateTime::TIMESTAMP_FULL);
    String message;
    serializeJson(doc, message);
    mqtt.publish(topic, message);

    vTaskDelay(xDelay);
  }
}