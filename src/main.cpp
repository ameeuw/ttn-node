#include <include.h>
#include "RTClib.h"

RTC_DS1307 rtc;

PicoMQTT::Server mqtt;

void lmicTask(void *);
void mqttTask(void *);
void handleMsgTask(void *);
void sendMsgTask(void *);
void doWorkTask(void *);
void handleDownlinkMsgTask(void *parameter);
void handleMqttMsgTask(void *parameter);

qMessage txMessage;
QueueHandle_t xQueue = xQueueCreate(10, sizeof(struct qMessage *));
extern QueueHandle_t downlinkQueue;
QueueHandle_t mqttQueue = xQueueCreate(10, sizeof(struct mqttMessage *));

void initTasks(void)
{
  xTaskCreatePinnedToCore(
      lmicTask,    /* Task function. */
      "LMIC Task", /* String with name of task. */
      30000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      1,           /* Priority of the task. */
      NULL,        /* Task handle. */
      1            /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      mqttTask,    /* Task function. */
      "MQTT Task", /* String with name of task. */
      20000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      1,           /* Priority of the task. */
      NULL,        /* Task handle. */
      0            /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      handleMsgTask,         /* Task function. */
      "Handle Message Task", /* String with name of task. */
      10000,                 /* Stack size in words. */
      NULL,                  /* Parameter passed as input of the task */
      2,                     /* Priority of the task. */
      NULL,                  /* Task handle. */
      1                      /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      sendMsgTask,         /* Task function. */
      "Send Message Task", /* String with name of task. */
      10000,               /* Stack size in words. */
      NULL,                /* Parameter passed as input of the task */
      3,                   /* Priority of the task. */
      NULL,                /* Task handle. */
      1                    /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      doWorkTask,    /* Task function. */
      "DoWork Task", /* String with name of task. */
      10000,         /* Stack size in words. */
      NULL,          /* Parameter passed as input of the task */
      3,             /* Priority of the task. */
      NULL,          /* Task handle. */
      1              /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      handleDownlinkMsgTask,  /* Task function. */
      "Handle Downlink Task", /* String with name of task. */
      10000,                  /* Stack size in words. */
      NULL,                   /* Parameter passed as input of the task */
      2,                      /* Priority of the task. */
      NULL,                   /* Task handle. */
      1                       /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      handleMqttMsgTask,  /* Task function. */
      "Handle Mqtt Task", /* String with name of task. */
      10000,              /* Stack size in words. */
      NULL,               /* Parameter passed as input of the task */
      1,                  /* Priority of the task. */
      NULL,               /* Task handle. */
      1                   /* Pinned CPU core. */
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
  Serial.print("Allocating memory. Free heap: ");
  Serial.println(xPortGetFreeHeapSize());
  mqttMessage *ptxMqttMessage = (mqttMessage *)pvPortMalloc(sizeof(mqttMessage));
  if (ptxMqttMessage == NULL)
  {
    Serial.println(F("Failed to allocate heap memory."));
    Serial.printf("topic: '%s'; payload: %s\n", topic, payload);
  }
  else
  {
    Serial.println("Forwarding message");
    ptxMqttMessage->topic = topic;
    ptxMqttMessage->payload = payload;
    xQueueSend(mqttQueue, &ptxMqttMessage, (TickType_t)0);
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
#else
  WiFi.softAP("MyESP32AP");
#endif

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
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

  // put your setup code here, to run once:
  setupLmic();
  initMqtt();
  initTasks();
}

void loop() {}

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

static volatile uint16_t counter_ = 0;

uint16_t getCounterValue()
{
  // Increments counter and returns the new value.
  return ++counter_;
}

void resetCounter()
{
  // Reset counter to 0
  counter_ = 0;
}

void doWorkTask(void *parameter)
{
  uint8_t count = 0;
  const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
  while (true)
  {
    Serial.printf("\ndoWorkTask running on core: %d\n", xPortGetCoreID());
    if (count == 3)
    {
      if (LMIC.devaddr != 0 && (LMIC.opmode & OP_TXRXPEND))
      {
        co2 co2Payload = {
            400,
            123,
            rtc.now().unixtime(),
            getCounterValue()};

        uint8_t fPort = 14;
        scheduleUplink(fPort, (uint8_t *)&co2Payload, sizeof(co2Payload), false);
      }
      else
      {
        Serial.println("Cannot schedule uplink... too much going on");
      }
      count = 0;
    }
    else
    {
      processWork(millis(), getCounterValue());
    }
    count++;
    vTaskDelay(xDelay);
  }
}

void sendMsgTask(void *parameter)
{
  uint lastMsg = 0;
  uint8_t msgCounter = 0;
  const TickType_t xDelay = 25000 / portTICK_PERIOD_MS;

  while (true)
  {
    DateTime time = rtc.now();
    Serial.println(String("Time:\t") + time.timestamp(DateTime::TIMESTAMP_FULL));

    Serial.printf("sendMsgTask running on core: %d\n", xPortGetCoreID());
    Serial.printf("last/now: %d / %d\n", lastMsg, millis());

    qMessage *ptxqMessage = (qMessage *)pvPortMalloc(sizeof(qMessage));
    if (ptxqMessage == NULL)
    {
      Serial.println(F("Failed to allocate heap memory."));
    }
    else
    {
      Serial.println("Sending message...");
      ptxqMessage->id = msgCounter;
      String msg = "This is a test message...";
      ptxqMessage->length = msg.length();
      memcpy(ptxqMessage->payload, msg.c_str(), ptxqMessage->length);
      xQueueSend(xQueue, &ptxqMessage, (TickType_t)0);

      msgCounter++;
      lastMsg = millis();
    }

    vTaskDelay(xDelay);
  }
}

void handleMsgTask(void *parameter)
{
  struct qMessage *prxqMessage;
  while (true)
  {
    // Pend on new message in queue and forward it to the corresponding handler
    if (xQueueReceive(xQueue, &(prxqMessage), portMAX_DELAY))
    {
      Serial.printf("handleMsgTask running on core: %d\n", xPortGetCoreID());
      Serial.println("Received xQueue");
      Serial.print("id: ");
      Serial.println((uint8_t)prxqMessage->id);
      Serial.print("length: ");
      Serial.println((uint8_t)prxqMessage->length);
      Serial.print("payload: ");
      for (uint8_t i = 0; i < (uint8_t)prxqMessage->length; i++)
      {
        Serial.print(prxqMessage->payload[i]);
      }
      Serial.println("");
      vPortFree(prxqMessage);
    }
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
      Serial.print("\nhandleDownlinkMsgTask running on core: ");
      Serial.println(xPortGetCoreID());
      Serial.println("Received downlinkQueue");
      Serial.printf("fport: %d; length: %d\n", prxdownlinkMessage->fport, prxdownlinkMessage->length);
      Serial.print("payload: ");
      for (uint8_t i = 0; i < (uint8_t)prxdownlinkMessage->length; i++)
      {
        Serial.print(prxdownlinkMessage->data[i]);
      }

      if (prxdownlinkMessage->fport == cmdPort && prxdownlinkMessage->length == 1 && prxdownlinkMessage->data[0] == resetCmd)
      {
        serial.println("Reset cmd received.");
        resetCounter();
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

void handleMqttMsgTask(void *parameter)
{
  mqttMessage *prxMqttMessage;

  while (true)
  {
    // Pend on new message in queue and forward it to the corresponding handler
    if (xQueueReceive(mqttQueue, &(prxMqttMessage), portMAX_DELAY))
    {
      Serial.println("handleMqttMsgTask running on core: " + xPortGetCoreID());
      Serial.println("Received mqttQueue");
      String nodeName = mqtt.get_topic_element(prxMqttMessage->topic.c_str(), 1);
      Serial.printf("topic: '%s'; payload: %s; nodeName: %s\n", prxMqttMessage->topic, prxMqttMessage->payload, nodeName);
      vPortFree(prxMqttMessage);
      Serial.print("Freed heap. Free heap: ");
      Serial.println(xPortGetFreeHeapSize());
    }
  }
}