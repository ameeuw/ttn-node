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

#ifdef USE_RTC
#include "RTClib.h"
RTC_DS1307 rtc;
#else
#include "TimeLib.h"
#endif

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

/*
**************************************************
*              Components
**************************************************
*/
#include "power.h"
#include "gps.h"
#include "utils.h"
#include "lmic-node.h"
#include "http_client.h"
#include "power.h"
#include "parsers.h"

#endif
