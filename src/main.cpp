#include <include.h>

PicoMQTT::Server mqtt;

void setup()
{
  // put your setup code here, to run once:
  setupLmic();

  // Subscribe to a topic and attach a callback
  mqtt.subscribe("tele/+/SENSOR", [](const char *topic, const char *payload)
                 {
        // payload might be binary, but PicoMQTT guarantees that it's zero-terminated
        Serial.printf("Received message in topic '%s': %s\n", topic, payload); });

  mqtt.begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
  os_runloop_once();
  mqtt.loop();
}