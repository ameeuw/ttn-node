#include <include.h>

PicoMQTT::Server mqtt;

void lmicTask(void *);
void mqttTask(void *);
void handleMsgTask(void *);
void sendMsgTask(void *);

void handleXQueueMessage(qMessage *rxMsg);

qMessage txMessage;
QueueHandle_t xQueue = xQueueCreate(10, sizeof(struct qMessage *));

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
  const TickType_t xDelay = 0 / portTICK_PERIOD_MS;
  while (true)
  {
    mqtt.loop();
    vTaskDelay(xDelay);
  }
}

void sendMsgTask(void *parameter)
{
  uint lastMsg = 0;
  uint8_t msgCounter = 0;
  const TickType_t xDelay = 60000 / portTICK_PERIOD_MS;

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
      handleXQueueMessage(pxRxedMessage);
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