#include <include.h>

PicoMQTT::Server mqtt;

void lmicTask(void *);
void mqttTask(void *);
void handleMsgTask(void *);
void sendMsgTask(void *);
void doWorkTask(void *);
void handleDownlinkMsgTask(void *parameter);

void handleXQueueMessage(qMessage *rxMsg);

qMessage txMessage;
QueueHandle_t xQueue = xQueueCreate(10, sizeof(struct qMessage *));
extern QueueHandle_t downlinkQueue;

void initTasks(void)
{
  xTaskCreatePinnedToCore(
      lmicTask,    /* Task function. */
      "LMIC Task", /* String with name of task. */
      30000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      1,           /* Priority of the task. */
      NULL,        /* Task handle. */
      0            /* Pinned CPU core. */
  );

  xTaskCreatePinnedToCore(
      mqttTask,    /* Task function. */
      "MQTT Task", /* String with name of task. */
      20000,       /* Stack size in words. */
      NULL,        /* Parameter passed as input of the task */
      2,           /* Priority of the task. */
      NULL,        /* Task handle. */
      1            /* Pinned CPU core. */
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
      0                    /* Pinned CPU core. */
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
}

void forwardMqttToQueue(const char *topic, const char *payload)
{
  Serial.printf("Received message in topic '%s': %s\n", topic, payload);
  struct qMessage *pxMessage;

  Serial.println("Forwarding message...");
  txMessage.id = 127;
  txMessage.length = strlen(topic);
  for (uint8_t i = 0; i < txMessage.length; i++)
  {
    txMessage.payload[i] = topic[i];
  }
  pxMessage = &txMessage;
  xQueueSend(xQueue, &pxMessage, (TickType_t)0);
}

void initMqtt()
{
  // Subscribe to a topic and attach a callback
  mqtt.subscribe("tele/+/SENSOR", forwardMqttToQueue);
  mqtt.begin();
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
  const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;
  while (true)
  {
    Serial.print("\ndoWorkTask running on core: ");
    Serial.println(xPortGetCoreID());
    uint16_t counterValue = getCounterValue();
    processWork(millis(), counterValue);
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
    Serial.print("\nsendMsgTask running on core: ");
    Serial.println(xPortGetCoreID());
    Serial.print(lastMsg);
    Serial.print(" / ");
    Serial.println(millis());
    struct qMessage *pxMessage;

    Serial.println("Sending message...");
    txMessage.id = msgCounter;
    msgCounter++;

    String msg = "This is a test message...";
    txMessage.length = msg.length();
    for (uint8_t i = 0; i < txMessage.length; i++)
    {
      txMessage.payload[i] = msg[i];
    }

    pxMessage = &txMessage;
    xQueueSend(xQueue, &pxMessage, (TickType_t)0);
    lastMsg = millis();
    vTaskDelay(xDelay);
  }
}

void handleMsgTask(void *parameter)
{
  struct qMessage *pxRxedMessage;
  while (true)
  {
    // Pend on new message in queue and forward it to the corresponding handler
    if (xQueueReceive(xQueue, &(pxRxedMessage), portMAX_DELAY))
    {
      Serial.print("\nhandleMsgTask running on core: ");
      Serial.println(xPortGetCoreID());
      Serial.println("Received xQueue");
      handleXQueueMessage(pxRxedMessage);
    }
  }
}

void handleDownlinkMsgTask(void *parameter)
{
  struct linkMessage *pdownlinkRxedMessage;

  const uint8_t cmdPort = 100;
  const uint8_t resetCmd = 0xC0;

  while (true)
  {
    // Pend on new message in queue and forward it to the corresponding handler
    if (xQueueReceive(downlinkQueue, &(pdownlinkRxedMessage), portMAX_DELAY))
    {
      Serial.print("\nhandleDownlinkMsgTask running on core: ");
      Serial.println(xPortGetCoreID());
      Serial.println("Received downlinkQueue");
      Serial.print("fport: ");
      Serial.println((uint8_t)pdownlinkRxedMessage->fport);
      Serial.print("length: ");
      Serial.println((uint8_t)pdownlinkRxedMessage->length);
      Serial.print("payload: ");
      for (uint8_t i = 0; i < (uint8_t)pdownlinkRxedMessage->length; i++)
      {
        Serial.print(pdownlinkRxedMessage->data[i]);
      }

      if (pdownlinkRxedMessage->fport == cmdPort && pdownlinkRxedMessage->length == 1 && pdownlinkRxedMessage->data[0] == resetCmd)
      {
        serial.println("Reset cmd received.");
        resetCounter();
      }
    }
  }
}

void handleXQueueMessage(qMessage *rxMsg)
{
  Serial.println("Received message in handleMsgTask");
  Serial.print("id: ");
  Serial.println((uint8_t)rxMsg->id);
  Serial.print("length: ");
  Serial.println((uint8_t)rxMsg->length);
  Serial.print("payload: ");
  for (uint8_t i = 0; i < (uint8_t)rxMsg->length; i++)
  {
    Serial.print(rxMsg->payload[i]);
  }
  Serial.println("");
  Serial.println(String((uint8_t)rxMsg->id) + " : " + String(rxMsg->payload));
}