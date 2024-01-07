#include "include.h"

// Tasmota Node registry
std::map<String, tasmotaNode> clientRegistry;

void updateClientRegistry()
{
    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;
    memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    // Iterate of all stations inside the adapter_sta_list
    for (esp_netif_sta_info_t station : adapter_sta_list.sta)
    {
        String mac = "";
        for (int i = 0; i < 6; i++)
        {
            char hex[3];
            sprintf(hex, "%02X", station.mac[i]);
            mac += hex;
        }
        if (mac != "000000000000")
        {
            String ip = ip4addr_ntoa((ip4_addr_t *)&(station.ip));

            if (clientRegistry.find(mac.c_str()) != clientRegistry.end())
            {
            }
            else
            {

                Serial.printf("Adding node %s (IP: %s) to registry", mac, ip);
                String hostname = "http://" + ip;
                String topic = hostname;
                tasmotaNode node = {hostname, ip, topic};
                clientRegistry[mac.c_str()] = node;
            }
        }
    }
}

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
    for (auto const &pair : clientRegistry)
    {
        // Iterate of all stations inside the clientRegistry
        String payload = fetchPayload(pair.second.hostname);
        if (payload != "")
        {
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            const JsonObject status = doc["StatusSNS"];
            if (status.containsKey("TRACER"))
            {
                tracerStruct tracerPayload = parseStruct<tracerStruct>(status, counterValue);
                Serial.print("Sending tracer telemetry at t=");
                Serial.print(tracerPayload.t);
                Serial.println(" ms");
                uint8_t fPort = tracerStruct::fport;
                // scheduleUplink(fPort, (uint8_t *)&tracerPayload, sizeof(tracerPayload));
            }
            else if (status.containsKey("meter"))
            {
                meterStruct meterPayload = parseStruct<meterStruct>(status, counterValue);
                Serial.print("Sending meter telemetry at t=");
                Serial.print(meterPayload.t);
                Serial.println(" ms");
                uint8_t fPort = meterStruct::fport;
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
}

void setupWeb()
{
}