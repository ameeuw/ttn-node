#include <include.h>

#include "IPv6Address.h"

#define HTTP_PORT 80
#define DNS_PORT 53
DNSServer dnsServer;
AsyncWebServer server(HTTP_PORT);

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
  WiFi.softAP("conode_dev", NULL, 12, false, 10);
  // WiFi.softAP("TTGO_v2_0002");
  if (!MDNS.begin("ludwig"))
    Serial.println("Error setting up MDNS responder!");
#elif defined(DEV3)
  WiFi.softAP("LOPY_0001");
#elif defined(DEV4)
  WiFi.softAP("LOPY_0002");
#elif defined(BOARD_TBEAM)
  WiFi.softAP("LUDWIG", WIFI_PASSWORD, 12, false, 10);
  if (!MDNS.begin("ludwig"))
    Serial.println("Error setting up MDNS responder!");
#elif defined(BOARD_DEV)
  WiFi.softAP("conode_dev", NULL, 12, false, 10);
  if (!MDNS.begin("ludwig"))
    Serial.println("Error setting up MDNS responder!");
#endif
  serial.begin(115200);

#ifdef BOARD_TBEAM
  startup_axp();
  axp_print();
  delay(3000);
  setup_axp();
  end_gps();
#endif // BOARD_TBEAM

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

  // Initialize SPIFFS
  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS Mount Failed");
  }
  else
  {
    Serial.println("Mounted LittleFS");
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
      Serial.print("FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
    }
  }
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              DynamicJsonDocument doc(2048);
              getStatusJson(doc);
              String message;
              serializeJson(doc, message);
              request->send(200, "text/json", message); });
  server.on("/command", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    char response[100];

    if (request->params() > 0) {
      AsyncWebParameter* p = request->getParam(0);
      if (strcmp(p->value().c_str(), "CMD_SET_TIME") == 0) {
        Serial.println("SETTING TIME");
        uint32_t timestamp = request->getParam(1)->value().toInt();
        Serial.print("Timestamp: ");
        Serial.println(timestamp);
        setTime(timestamp);
        Serial.println("SENDING RESPONSE.");
        sprintf(
          response,
          "%i",
          timestamp
        );
        request->send(200, "text/plain", response);
      } else {
        Serial.println("NO VALID COMMAND!");
        request->send(200, "text/plain", "NO VALID CMD!");
      }
    } });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String message = "";
              request->send(200, "text/plain", message); });
  server.serveStatic("/", LittleFS, "/");
  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.println("NotFound! Serving index.html!");
    request->send(LittleFS, "/index.html", "text/html"); });
  // AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();

#ifndef BOARD_DEV
  initLmic();
#endif // BOARD_DEV
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
    updateClientRegistry();
    publishStatusMessage();

    if (counter % 6 == 0)
    {
#ifdef BOARD_TBEAM
      updateGPS(counter);
#endif // BOARD_TBEAM

      updateNodeTime();
    }
    counter++;

    vTaskDelay(xDelay);
  }
}