#include <include.h>

#ifdef BOARD_TBEAM
#define HTTP_PORT 80
#define DNS_PORT 53
DNSServer dnsServer;
AsyncWebServer server(HTTP_PORT);
#endif // BOARD_TBEAM

// Task definitions
void statusTask(void *parameter);

void initTasks(void)
{

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

#ifdef BOARD_TBEAM
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("Mounted SPIFFS");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
      Serial.print("FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
    }
  }
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.serveStatic("/", SPIFFS, "/");
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.println("NotFound! Serving index.html!");
    request->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              DynamicJsonDocument doc(1024);
              getStatusJson(doc);
              String message;
              serializeJson(doc, message);
              request->send(200, "text/json", message); });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String message = "";
              request->send(200, "text/plain", message); });
  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
#endif // BOARD_TBEAM

  initLmic();
  initMqtt();
  initHelperTasks();
  initTasks();
}

void loop()
{
  dnsServer.processNextRequest();
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