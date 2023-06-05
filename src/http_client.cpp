#include "include.h"

meter parseMeterStruct(JsonObject doc, uint16_t counterValue);
tracer parseTracerStruct(JsonObject doc, uint16_t counterValue);

String fetchPayload(String serverName)
{
    HTTPClient http;
    String serverPath = serverName + "/cm?cmnd=status%2010";
    // Your Domain name with URL path or IP address with path
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();
    String payload = "";
    if (httpResponseCode > 0)
    {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    }
    else
    {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();

    return payload;
}

void collect(uint16_t counterValue)
{
    String serverName = "";

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;

    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    // Iterate of all stations inside the adapter_sta_list
    for (esp_netif_sta_info_t station : adapter_sta_list.sta)
    {
        Serial.print("MAC: ");
        for (int i = 0; i < 6; i++)
        {
            Serial.printf("%02X", station.mac[i]);
            if (i < 5)
                Serial.print(":");
        }
        String ip = ip4addr_ntoa((ip4_addr_t *)&(station.ip));
        Serial.print("\nIP: ");
        Serial.println(ip);
        serverName = "http://" + ip;
        Serial.print("\nServer: ");
        Serial.println(serverName);
        if (serverName != "" && serverName != "http://0.0.0.0")
        {
            String payload = fetchPayload(serverName);

            if (payload != "")
            {
                DynamicJsonDocument doc(1024);
                deserializeJson(doc, payload);
                const JsonObject status = doc["StatusSNS"];
                if (status.containsKey("TRACER"))
                {
                    tracer tracerPayload = parseTracerStruct(status, counterValue);
                    Serial.print("Sending tracer telemetry at t=");
                    Serial.print(tracerPayload.t);
                    Serial.println(" ms");
                    uint8_t fPort = 12;
                    // scheduleUplink(fPort, (uint8_t *)&tracerPayload, sizeof(tracerPayload));
                }
                else if (status.containsKey("meter"))
                {
                    meter meterPayload = parseMeterStruct(status, counterValue);
                    Serial.print("Sending meter telemetry at t=");
                    Serial.print(meterPayload.t);
                    Serial.println(" ms");
                    uint8_t fPort = 13;
                    // scheduleUplink(fPort, (uint8_t *)&meterPayload, sizeof(meterPayload));
                }
                else
                {
                    Serial.println("Cannot find compatible payload.");
                    Serial.println("Supported structs are: 'tracer', 'meter'");
                }
            }
            else
            {
                Serial.println("No payload");
            }
        }
        else
        {
            Serial.println("No client connected");
        }
    }
}

meter parseMeterStruct(JsonObject doc, uint16_t counterValue)
{
    meter meterPayload = {
        ((float)doc["ANALOG"]["Range"]) / 1000,
        doc["meter"]["power"],
        doc["meter"]["consumption"],
        millis(),
        counterValue,
    };
    return meterPayload;
}

tracer parseTracerStruct(JsonObject doc, uint16_t counterValue)
{
    tracer tracerPayload = {
        doc["TRACER"]["batteryTemperature"],
        doc["TRACER"]["batterySoc"],
        doc["TRACER"]["batteryVoltage"],
        doc["TRACER"]["batteryCurrent"],
        doc["TRACER"]["pvVoltage"],
        doc["TRACER"]["pvCurrent"],
        doc["TRACER"]["pvPower"],
        doc["TRACER"]["loadVoltage"],
        doc["TRACER"]["loadCurrent"],
        doc["TRACER"]["loadPower"],
        doc["TRACER"]["consumptionDay"],
        doc["TRACER"]["consumptionSum"],
        doc["TRACER"]["productionSum"],
        doc["TRACER"]["batteryMaxVoltage"],
        counterValue,
        millis(),
    };
    return tracerPayload;
}

co2 parseCo2Struct(JsonObject doc, uint16_t counterValue)
{
    return (co2){
        doc["CO2"],
        doc["ANALOG"]["Illuminance"],
        millis(),
        counterValue,
    };
}