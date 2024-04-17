#ifndef include_h
#define include_h

/*
**************************************************
*              Arduino Libraries
**************************************************
*/

#include <Arduino.h>
#include <PicoMQTT.h>

#include "esp_wifi.h"
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#ifdef BOARD_TBEAM
#include <TinyGPS++.h>
#endif // BOARD_TBEAM

#include <AsyncTCP.h>
#include <DNSServer.h>
#include "FS.h"
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>

#ifdef USE_RTC
#include "RTClib.h"
#endif

#include "TimeLib.h"

#include "lmic.h"
#include "hal/hal.h"

#ifdef USE_LED
#include "EasyLed.h"
#endif

/*
**************************************************
*              Typedefinitions
**************************************************
*/
#include <typedef.h>
#include BSFILE // Include Board Support File
#include "../keyfiles/lorawan-keys.h"

/*
**************************************************
*              Components
**************************************************
*/
#include "power.h"
#include "gps.h"
#include "utils.h"
#include "lmic-node.h"
#include "http-helpers.h"
#include "power.h"
#include "parsers.h"
#include "lmic-helpers.h"
#include "mqtt.h"

#endif
