#include <include.h>

// Task definitions
TaskHandle_t LmicTask;
void lmicTask(void *);
void statusTask(void *parameter);

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
      statusTask,    /* Task function. */
      "Status Task", /* String with name of task. */
      10000,         /* Stack size in words. */
      NULL,          /* Parameter passed as input of the task */
      1,             /* Priority of the task. */
      NULL,          /* Task handle. */
      1              /* Pinned CPU core. */
  );
}

void setup()
{
#if defined(DEV2)
  WiFi.softAP("TTGO_v2_0002");
#elif defined(DEV3)
  WiFi.softAP("LOPY_0001");
#elif defined(DEV4)
  WiFi.softAP("LOPY_0002");
#elif defined(BOARD_TBEAM)
  WiFi.softAP("LUDWIG", WIFI_PASSWORD, 12, false, 10);
  if (!MDNS.begin("ludwig"))
    Serial.println("Error setting up MDNS responder!");
#endif
  serial.begin(115200);

  startup_axp();
  axp_print();
  delay(3000);
  setup_axp();
  end_gps();

#ifdef USE_RTC
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
#endif

  setupLmic();
  initMqtt();
  initHelperTasks();
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

void statusTask(void *parameter)
{
  const TickType_t xDelay = 30000 / portTICK_PERIOD_MS;
  uint16_t counter = 0;

  while (true)
  {

    publishStatusMessage();

    if (counter % 6 == 0)
    {
      updateGPS(counter);
      updateNodeTime();
    }
    counter++;

    vTaskDelay(xDelay);
  }
}