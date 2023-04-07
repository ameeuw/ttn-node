#ifndef include_h
#define include_h

/*
**************************************************
*              Arduino Libraries
**************************************************
*/

#include <Arduino.h>

#include "esp_wifi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

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
#include "utils.h"
#include "lmic-node.h"

#endif
