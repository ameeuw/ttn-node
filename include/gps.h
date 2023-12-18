#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>
#include "include.h"

#ifdef ESP32
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#endif

#ifdef CUBECELL
#include "LoRaWan_APP.h"
#include "GPS_Air530.h"
#include "GPS_Air530Z.h"

extern Air530Class serialGPS;
#else
extern HardwareSerial serialGPS;
#endif

#define UTC_OFFSET 1 // Central European Time

#define GPS_RX 34
#define GPS_TX 12
#define GPS_INTERVAL 600
#define GPS_MOVE_DIST 25

// locations excludet for ttnmapper-upload
struct Geofence
{
    double lat;
    double lng;
    u_int radius;
};

const struct Geofence geofence[] = {
    {52.01910, 8.53100, 10000},  // Bielefeld
    {52.51704, 13.38886, 15000}, // Berlin
    {48.77845, 9.18001, 5000}    // Stuttgart
};

template <typename T>
extern void handlePayloadAndQueueUplink(const char *payload);

extern TinyGPSPlus gps;

extern SLEEP_VAR double last_lat;
extern SLEEP_VAR double last_lng;

void setup_gps();
void end_gps();
void gps_loop();
int getGPS();
void updateGPS(uint16_t counter);
bool gps_valid();
bool gps_moved(int meter);
uint8_t gps_geo();
uint8_t location_bin(uint8_t *txBuffer, uint8_t offset);
