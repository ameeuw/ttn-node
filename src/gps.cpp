#include "gps.h"

TinyGPSPlus gps;
#ifndef CUBECELL
HardwareSerial serialGPS(1);
#else
Air530Class serialGPS;
#endif

void setup_gps()
{
#ifndef CUBECELL
    serialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
#else
    digitalWrite(GPIO14, LOW); // VGPS ON
    serialGPS.begin();
#endif
}

void end_gps()
{
    serialGPS.end();
#ifdef CUBECELL
    digitalWrite(GPIO14, HIGH); // VGPS OFF
#endif
#ifdef ESP32
    digitalWrite(GPS_TX, HIGH);
    gpio_hold_en((gpio_num_t)GPS_TX);
#endif
}

void gps_loop()
{
    unsigned long lock = millis() + 2;
    while (serialGPS.available() > 0 && millis() < lock)
    {
        char c = serialGPS.read();
        gps.encode(c);
#ifdef DEBUG
        Serial.print(c);
#endif
    }
}

int getGPS()
{
    Serial.println("GPS-loop");
    unsigned long time = millis() + 1200;
    while (!gps_valid() && time > millis())
        gps_loop();

    Serial.flush();
    Serial.print("GPS-end: ");

    if (!gps_valid()) // no GPS
    {
        Serial.println("no fix");
        return 0;
    }
    if (gps_geo()) // Geofence
    {
        Serial.println("geofence");
        return 3;
    }
    if (!gps_moved(GPS_MOVE_DIST)) // no movement
    {
        Serial.println("no movement");
        Serial.println("sending anyway");
        return 1;
        // return 2;
    }

    Serial.println("ok");
    return 1; // GPS ok
}

void updateGPS(uint16_t counter)
{
    setup_gps();
    int gpsStatus = getGPS();
    if (gpsStatus == 0)
        // Increase voltage for GPS to get a fix
        axp_gps(1);
    else
        // Reduce voltage for GPS to save power
        axp_gps(2);

    if (gpsStatus == 1)
    {
        // Set Time from GPS data
        setTime(
            gps.time.hour(),
            gps.time.minute(),
            gps.time.second(),
            gps.date.day(),
            gps.date.month(),
            gps.date.year());
        // Adjust to Time Zone time by offset value
        adjustTime(UTC_OFFSET * SECS_PER_HOUR);

        String gpsPayload =
            "{\"latitude\": " + String(gps.location.lat()) +
            ", \"longitude\": " + String(gps.location.lng()) +
            ", \"altitude\": " + String(gps.altitude.meters()) +
            ", \"speed\": " + String(gps.speed.kmph()) +
            "}";
        handlePayloadAndQueueUplink<gpsStruct>(gpsPayload.c_str());
    }
    else
    {
        Serial.println("No GPS");
    }
}

bool gps_valid()
{
    if (gps.location.isValid() && gps.hdop.isValid() && gps.altitude.isValid() && gps.location.age() <= 1200 && gps.hdop.age() <= 1200 && gps.altitude.age() <= 1200)
        return true;
    else
        return false;
}

SLEEP_VAR double last_lat;
SLEEP_VAR double last_lng;
bool gps_moved(int meter)
{
    if (gps.distanceBetween(gps.location.lat(), gps.location.lng(), last_lat, last_lng) < meter)
        return false;
    else
    {
        last_lat = gps.location.lat();
        last_lng = gps.location.lng();
        return true;
    }
}

uint8_t gps_geo()
{
    for (int i = 0; i < (sizeof(geofence) / sizeof(Geofence)); i++)
    {
        if (gps.distanceBetween(gps.location.lat(), gps.location.lng(), geofence[i].lat, geofence[i].lng) < geofence[i].radius)
        {
            return i + 1;
        }
    }
    return 0;
}
