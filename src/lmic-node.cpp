#include "lmic-node.h"

uint16_t getCounterValue();
void processWork(ostime_t doWorkJobTimeStamp, uint16_t counterValue);
void sendStruct(ostime_t doWorkJobTimeStamp, uint16_t counterValue);
void collectAndSend(ostime_t doWorkJobTimeStamp, uint16_t counterValue);

const uint8_t payloadBufferLength = 4; // Adjust to fit max payload length

uint8_t payloadBuffer[payloadBufferLength];
static osjob_t doWorkJob;
uint32_t doWorkIntervalSeconds = DO_WORK_INTERVAL_SECONDS; // Change value in platformio.ini

// Set LoRaWAN keys defined in lorawan-keys.h.
#ifdef OTAA_ACTIVATION
static const u1_t PROGMEM DEVEUI[8] = {OTAA_DEVEUI};
static const u1_t PROGMEM APPEUI[8] = {OTAA_APPEUI};
static const u1_t PROGMEM APPKEY[16] = {OTAA_APPKEY};
// Below callbacks are used by LMIC for reading above values.
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }
#else
                                                           // ABP activation
static const u4_t DEVADDR = ABP_DEVADDR;
static const PROGMEM u1_t NWKSKEY[16] = {ABP_NWKSKEY};
static const u1_t PROGMEM APPSKEY[16] = {ABP_APPSKEY};
// Below callbacks are not used be they must be defined.
void os_getDevEui(u1_t *buf) {}
void os_getArtEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}
#endif

int16_t getSnrTenfold()
{
    // Returns ten times the SNR (dB) value of the last received packet.
    // Ten times to prevent the use of float but keep 1 decimal digit accuracy.
    // Calculation per SX1276 datasheet rev.7 §6.4, SX1276 datasheet rev.4 §6.4.
    // LMIC.snr contains value of PacketSnr, which is 4 times the actual SNR value.
    return (LMIC.snr * 10) / 4;
}

int16_t getRssi(int8_t snr)
{
    // Returns correct RSSI (dBm) value of the last received packet.
    // Calculation per SX1276 datasheet rev.7 §5.5.5, SX1272 datasheet rev.4 §5.5.5.

#define RSSI_OFFSET 64
#define SX1276_FREQ_LF_MAX 525000000 // per datasheet 6.3
#define SX1272_RSSI_ADJUST -139
#define SX1276_RSSI_ADJUST_LF -164
#define SX1276_RSSI_ADJUST_HF -157

    int16_t rssi;

#ifdef MCCI_LMIC

    rssi = LMIC.rssi - RSSI_OFFSET;

#else
    int16_t rssiAdjust;
#ifdef CFG_sx1276_radio
    if (LMIC.freq > SX1276_FREQ_LF_MAX)
    {
        rssiAdjust = SX1276_RSSI_ADJUST_HF;
    }
    else
    {
        rssiAdjust = SX1276_RSSI_ADJUST_LF;
    }
#else
    // CFG_sx1272_radio
    rssiAdjust = SX1272_RSSI_ADJUST;
#endif

    // Revert modification (applied in lmic/radio.c) to get PacketRssi.
    int16_t packetRssi = LMIC.rssi + 125 - RSSI_OFFSET;
    if (snr < 0)
    {
        rssi = rssiAdjust + packetRssi + snr;
    }
    else
    {
        rssi = rssiAdjust + (16 * packetRssi) / 15;
    }
#endif

    return rssi;
}

void printEvent(ostime_t timestamp,
                const char *const message,
                PrintTarget target = PrintTarget::All,
                bool clearDisplayStatusRow = true,
                bool eventLabel = false)
{
#ifdef USE_DISPLAY
    if (target == PrintTarget::All || target == PrintTarget::Display)
    {
        display.clearLine(TIME_ROW);
        display.setCursor(COL_0, TIME_ROW);
        display.print(F("Time:"));
        display.print(timestamp);
        display.clearLine(EVENT_ROW);
        if (clearDisplayStatusRow)
        {
            display.clearLine(STATUS_ROW);
        }
        display.setCursor(COL_0, EVENT_ROW);
        display.print(message);
    }
#endif

#ifdef USE_SERIAL
    // Create padded/indented output without using printf().
    // printf() is not default supported/enabled in each Arduino core.
    // Not using printf() will save memory for memory constrainted devices.
    String timeString(timestamp);
    uint8_t len = timeString.length();
    uint8_t zerosCount = TIMESTAMP_WIDTH > len ? TIMESTAMP_WIDTH - len : 0;

    if (target == PrintTarget::All || target == PrintTarget::Serial)
    {
        printChars(serial, '0', zerosCount);
        serial.print(timeString);
        serial.print(":  ");
        if (eventLabel)
        {
            serial.print(F("Event: "));
        }
        serial.println(message);
    }
#endif
}

void printEvent(ostime_t timestamp,
                ev_t ev,
                PrintTarget target = PrintTarget::All,
                bool clearDisplayStatusRow = true)
{
#if defined(USE_DISPLAY) || defined(USE_SERIAL)
    printEvent(timestamp, lmicEventNames[ev], target, clearDisplayStatusRow, true);
#endif
}

void printSessionKeys()
{
#if defined(USE_SERIAL) && defined(MCCI_LMIC)
    u4_t networkId = 0;
    devaddr_t deviceAddress = 0;
    u1_t networkSessionKey[16];
    u1_t applicationSessionKey[16];
    LMIC_getSessionKeys(&networkId, &deviceAddress,
                        networkSessionKey, applicationSessionKey);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Network Id: "));
    serial.println(networkId, DEC);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Device Address: "));
    serial.println(deviceAddress, HEX);

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Application Session Key: "));
    printHex(serial, applicationSessionKey, 16, true, '-');

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Network Session Key:     "));
    printHex(serial, networkSessionKey, 16, true, '-');
#endif
}

void printFrameCounters(PrintTarget target = PrintTarget::All)
{
#ifdef USE_DISPLAY
    if (target == PrintTarget::Display || target == PrintTarget::All)
    {
        display.clearLine(FRMCNTRS_ROW);
        display.setCursor(COL_0, FRMCNTRS_ROW);
        display.print(F("Up:"));
        display.print(LMIC.seqnoUp);
        display.print(F(" Dn:"));
        display.print(LMIC.seqnoDn);
    }
#endif

#ifdef USE_SERIAL
    if (target == PrintTarget::Serial || target == PrintTarget::All)
    {
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Up: "));
        serial.print(LMIC.seqnoUp);
        serial.print(F(",  Down: "));
        serial.println(LMIC.seqnoDn);
    }
#endif
}

void printDownlinkInfo(void)
{
#if defined(USE_SERIAL) || defined(USE_DISPLAY)

    uint8_t dataLength = LMIC.dataLen;
    // bool ackReceived = LMIC.txrxFlags & TXRX_ACK;

    int16_t snrTenfold = getSnrTenfold();
    int8_t snr = snrTenfold / 10;
    int8_t snrDecimalFraction = snrTenfold % 10;
    int16_t rssi = getRssi(snr);

    uint8_t fPort = 0;
    if (LMIC.txrxFlags & TXRX_PORT)
    {
        fPort = LMIC.frame[LMIC.dataBeg - 1];
    }

#ifdef USE_DISPLAY
    display.clearLine(EVENT_ROW);
    display.setCursor(COL_0, EVENT_ROW);
    display.print(F("RX P:"));
    display.print(fPort);
    if (dataLength != 0)
    {
        display.print(" Len:");
        display.print(LMIC.dataLen);
    }
    display.clearLine(STATUS_ROW);
    display.setCursor(COL_0, STATUS_ROW);
    display.print(F("RSSI"));
    display.print(rssi);
    display.print(F(" SNR"));
    display.print(snr);
    display.print(".");
    display.print(snrDecimalFraction);
#endif

#ifdef USE_SERIAL
    printSpaces(serial, MESSAGE_INDENT);
    serial.println(F("Downlink received"));

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("RSSI: "));
    serial.print(rssi);
    serial.print(F(" dBm,  SNR: "));
    serial.print(snr);
    serial.print(".");
    serial.print(snrDecimalFraction);
    serial.println(F(" dB"));

    printSpaces(serial, MESSAGE_INDENT);
    serial.print(F("Port: "));
    serial.println(fPort);

    if (dataLength != 0)
    {
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Length: "));
        serial.println(LMIC.dataLen);
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("Data: "));
        printHex(serial, LMIC.frame + LMIC.dataBeg, LMIC.dataLen, true, ' ');
    }
#endif
#endif
}

void printHeader(void)
{
#ifdef USE_DISPLAY
    display.clear();
    display.setCursor(COL_0, HEADER_ROW);
    display.print(F("LMIC-node"));
#ifdef ABP_ACTIVATION
    display.drawString(ABPMODE_COL, HEADER_ROW, "ABP");
#endif
#ifdef CLASSIC_LMIC
    display.drawString(CLMICSYMBOL_COL, HEADER_ROW, "*");
#endif
    display.drawString(COL_0, DEVICEID_ROW, deviceId);
    display.setCursor(COL_0, INTERVAL_ROW);
    display.print(F("Interval:"));
    display.print(doWorkIntervalSeconds);
    display.print("s");
#endif

#ifdef USE_SERIAL
    serial.println(F("\n\nLMIC-node\n"));
    serial.print(F("Device-id:     "));
    serial.println(deviceId);
    serial.print(F("LMIC library:  "));
#ifdef MCCI_LMIC
    serial.println(F("MCCI"));
#else
    serial.println(F("Classic [Deprecated]"));
#endif
    serial.print(F("Activation:    "));
#ifdef OTAA_ACTIVATION
    serial.println(F("OTAA"));
#else
    serial.println(F("ABP"));
#endif
#if defined(LMIC_DEBUG_LEVEL) && LMIC_DEBUG_LEVEL > 0
    serial.print(F("LMIC debug:    "));
    serial.println(LMIC_DEBUG_LEVEL);
#endif
    serial.print(F("Interval:      "));
    serial.print(doWorkIntervalSeconds);
    serial.println(F(" seconds"));
    if (activationMode == ActivationMode::OTAA)
    {
        serial.println();
    }
#endif
}

void initLmic(bit_t adrEnabled = 1,
              dr_t abpDataRate = DefaultABPDataRate,
              s1_t abpTxPower = DefaultABPTxPower)
{
    // ostime_t timestamp = os_getTime();

    // Initialize LMIC runtime environment
    os_init();
    // Reset MAC state
    LMIC_reset();

#ifdef ABP_ACTIVATION
    setAbpParameters(abpDataRate, abpTxPower);
#endif

    // Enable or disable ADR (data rate adaptation).
    // Should be turned off if the device is not stationary (mobile).
    // 1 is on, 0 is off.
    LMIC_setAdrMode(adrEnabled);

    if (activationMode == ActivationMode::OTAA)
    {
#if defined(CFG_us915) || defined(CFG_au915)
        // NA-US and AU channels 0-71 are configured automatically
        // but only one group of 8 should (a subband) should be active
        // TTN recommends the second sub band, 1 in a zero based count.
        // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
        LMIC_selectSubBand(1);
#endif
    }

// Relax LMIC timing if defined
#if defined(LMIC_CLOCK_ERROR_PPM)
    uint32_t clockError = 0;
#if LMIC_CLOCK_ERROR_PPM > 0
#if defined(MCCI_LMIC) && LMIC_CLOCK_ERROR_PPM > 4000
// Allow clock error percentage to be > 0.4%
#define LMIC_ENABLE_arbitrary_clock_error 1
#endif
    clockError = (LMIC_CLOCK_ERROR_PPM / 100) * (MAX_CLOCK_ERROR / 100) / 100;
    LMIC_setClockError(clockError);
#endif

#ifdef USE_SERIAL
    serial.print(F("Clock Error:   "));
    serial.print(LMIC_CLOCK_ERROR_PPM);
    serial.print(" ppm (");
    serial.print(clockError);
    serial.println(")");
#endif
#endif

#ifdef MCCI_LMIC
    // Register a custom eventhandler and don't use default onEvent() to enable
    // additional features (e.g. make EV_RXSTART available). User data pointer is omitted.
    LMIC_registerEventCb(&onLmicEvent, nullptr);
#endif
}

#ifdef MCCI_LMIC
void onLmicEvent(void *pUserData, ev_t ev)
#else
void onEvent(ev_t ev)
#endif
{
    // LMIC event handler
    ostime_t timestamp = os_getTime();

    switch (ev)
    {
#ifdef MCCI_LMIC
    // Only supported in MCCI LMIC library:
    case EV_RXSTART:
        // Do not print anything for this event or it will mess up timing.
        break;

    case EV_TXSTART:
        setTxIndicatorsOn();
        printEvent(timestamp, ev);
        break;

    case EV_JOIN_TXCOMPLETE:
    case EV_TXCANCELED:
        setTxIndicatorsOn(false);
        printEvent(timestamp, ev);
        break;
#endif
    case EV_JOINED:
        setTxIndicatorsOn(false);
        printEvent(timestamp, ev);
        printSessionKeys();

        // Disable link check validation.
        // Link check validation is automatically enabled
        // during join, but because slow data rates change
        // max TX size, it is not used in this example.
        LMIC_setLinkCheckMode(0);

        // The doWork job has probably run already (while
        // the node was still joining) and have rescheduled itself.
        // Cancel the next scheduled doWork job and re-schedule
        // for immediate execution to prevent that any uplink will
        // have to wait until the current doWork interval ends.
        os_clearCallback(&doWorkJob);
        os_setCallback(&doWorkJob, doWorkCallback);
        break;

    case EV_TXCOMPLETE:
        // Transmit completed, includes waiting for RX windows.
        setTxIndicatorsOn(false);
        printEvent(timestamp, ev);
        printFrameCounters();

        // Check if downlink was received
        if (LMIC.dataLen != 0 || LMIC.dataBeg != 0)
        {
            uint8_t fPort = 0;
            if (LMIC.txrxFlags & TXRX_PORT)
            {
                fPort = LMIC.frame[LMIC.dataBeg - 1];
            }
            printDownlinkInfo();
            processDownlink(timestamp, fPort, LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        }
        break;
    case EV_LINK_DEAD:
        printEvent(timestamp, ev);
        ESP.restart();
        break;

    // Below events are printed only.
    case EV_SCAN_TIMEOUT:
    case EV_BEACON_FOUND:
    case EV_BEACON_MISSED:
    case EV_BEACON_TRACKED:
    case EV_RFU1: // This event is defined but not used in code
    case EV_JOINING:
    case EV_JOIN_FAILED:
    case EV_REJOIN_FAILED:
    case EV_LOST_TSYNC:
    case EV_RESET:
    case EV_RXCOMPLETE:
    case EV_LINK_ALIVE:
#ifdef MCCI_LMIC
    // Only supported in MCCI LMIC library:
    case EV_SCAN_FOUND: // This event is defined but not used in code
#endif
        printEvent(timestamp, ev);
        break;

    default:
        printEvent(timestamp, "Unknown Event");
        break;
    }
}

static void doWorkCallback(osjob_t *job)
{
    // Event hander for doWorkJob. Gets called by the LMIC scheduler.
    // The actual work is performed in function processWork() which is called below.

    ostime_t timestamp = os_getTime();
#ifdef USE_SERIAL
    serial.println();
    printEvent(timestamp, "doWork job started", PrintTarget::Serial);
#endif

    // Do the work that needs to be performed.
    uint16_t counterValue = getCounterValue();
    if (counterValue % 2 == 0)
    {
        collectAndSend(timestamp, counterValue);
    }
    else
    {
        processWork(timestamp, counterValue);
    }

    // This job must explicitly reschedule itself for the next run.
    ostime_t startAt = timestamp + sec2osticks((int64_t)doWorkIntervalSeconds);
    os_setTimedCallback(&doWorkJob, startAt, doWorkCallback);
}

lmic_tx_error_t scheduleUplink(uint8_t fPort, uint8_t *data, uint8_t dataLength, bool confirmed = false)
{
    // This function is called from the processWork() function to schedule
    // transmission of an uplink message that was prepared by processWork().
    // Transmission will be performed at the next possible time

    ostime_t timestamp = os_getTime();
    printEvent(timestamp, "Packet queued");

    lmic_tx_error_t retval = LMIC_setTxData2(fPort, data, dataLength, confirmed ? 1 : 0);
    timestamp = os_getTime();

    if (retval == LMIC_ERROR_SUCCESS)
    {
#ifdef CLASSIC_LMIC
        // For MCCI_LMIC this will be handled in EV_TXSTART
        setTxIndicatorsOn();
#endif
    }
    else
    {
        String errmsg;
#ifdef USE_SERIAL
        errmsg = "LMIC Error: ";
#ifdef MCCI_LMIC
        errmsg.concat(lmicErrorNames[abs(retval)]);
#else
        errmsg.concat(retval);
#endif
        printEvent(timestamp, errmsg.c_str(), PrintTarget::Serial);
#endif
#ifdef USE_DISPLAY
        errmsg = "LMIC Err: ";
        errmsg.concat(retval);
        printEvent(timestamp, errmsg.c_str(), PrintTarget::Display);
#endif
    }
    return retval;
}

static volatile uint16_t counter_ = 0;

uint16_t getCounterValue()
{
    // Increments counter and returns the new value.
    delay(50); // Fake this takes some time
    return ++counter_;
}

void resetCounter()
{
    // Reset counter to 0
    counter_ = 0;
}

void processWork(ostime_t doWorkJobTimeStamp, uint16_t counterValue)
{
    // This function is called from the doWorkCallback()
    // callback function when the doWork job is executed.

    // Uses globals: payloadBuffer and LMIC data structure.

    // This is where the main work is performed like
    // reading sensor and GPS data and schedule uplink
    // messages if anything needs to be transmitted.

    // Skip processWork if using OTAA and still joining.
    if (LMIC.devaddr != 0)
    {
        // Collect input data.
        // For simplicity LMIC-node uses a counter to simulate a sensor.
        // The counter is increased automatically by getCounterValue()
        // and can be reset with a 'reset counter' command downlink message.

        ostime_t timestamp = os_getTime();

#ifdef USE_DISPLAY
        // Interval and Counter values are combined on a single row.
        // This allows to keep the 3rd row empty which makes the
        // information better readable on the small display.
        display.clearLine(INTERVAL_ROW);
        display.setCursor(COL_0, INTERVAL_ROW);
        display.print("I:");
        display.print(doWorkIntervalSeconds);
        display.print("s");
        display.print(" Ctr:");
        display.print(counterValue);
#endif
#ifdef USE_SERIAL
        printEvent(timestamp, "Input data collected", PrintTarget::Serial);
        printSpaces(serial, MESSAGE_INDENT);
        serial.print(F("COUNTER value: "));
        serial.println(counterValue);
#endif

        // For simplicity LMIC-node will try to send an uplink
        // message every time processWork() is executed.

        // Schedule uplink message if possible
        if (LMIC.opmode & OP_TXRXPEND)
        {
// TxRx is currently pending, do not send.
#ifdef USE_SERIAL
            printEvent(timestamp, "Uplink not scheduled because TxRx pending", PrintTarget::Serial);
#endif
#ifdef USE_DISPLAY
            printEvent(timestamp, "UL not scheduled", PrintTarget::Display);
#endif
        }
        else
        {
            // Prepare uplink payload.
            uint8_t fPort = 10;
            payloadBuffer[0] = counterValue >> 8;
            payloadBuffer[1] = counterValue & 0xFF;
            uint8_t payloadLength = 2;

            scheduleUplink(fPort, payloadBuffer, payloadLength);
        }
    }
}

void sendCounterValue(uint16_t counterValue)
{
    // Prepare uplink payload.
    uint8_t fPort = 10;
    payloadBuffer[0] = counterValue >> 8;
    payloadBuffer[1] = counterValue & 0xFF;
    uint8_t payloadLength = 2;
    scheduleUplink(fPort, payloadBuffer, payloadLength);
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

void collectAndSend(ostime_t doWorkJobTimeStamp, uint16_t counterValue)
{
    if (LMIC.devaddr != 0)
    {
        // Collect input data.
        // For simplicity LMIC-node uses a counter to simulate a sensor.
        // The counter is increased automatically by getCounterValue()
        // and can be reset with a 'reset counter' command downlink message.

        ostime_t timestamp = os_getTime();

#ifdef USE_DISPLAY
        // Interval and Counter values are combined on a single row.
        // This allows to keep the 3rd row empty which makes the
        // information better readable on the small display.
        display.clearLine(INTERVAL_ROW);
        display.setCursor(COL_0, INTERVAL_ROW);
        display.print("Sending struct.");
        display.print(" Ctr:");
        display.print(counterValue);
#endif
#ifdef USE_SERIAL
        printEvent(timestamp, "Input data collected", PrintTarget::Serial);
        printSpaces(serial, MESSAGE_INDENT);
#endif

        // For simplicity LMIC-node will try to send an uplink
        // message every time processWork() is executed.

        // Schedule uplink message if possible
        if (LMIC.opmode & OP_TXRXPEND)
        {
// TxRx is currently pending, do not send.
#ifdef USE_SERIAL
            printEvent(timestamp, "Uplink not scheduled because TxRx pending", PrintTarget::Serial);
#endif
#ifdef USE_DISPLAY
            printEvent(timestamp, "UL not scheduled", PrintTarget::Display);
#endif
        }
        else
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
                if (serverName != "")
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
                            Serial.print("Sending telemetry at t=");
                            Serial.print(tracerPayload.t);
                            Serial.println(" ms");
                            uint8_t fPort = 12;
                            scheduleUplink(fPort, (uint8_t *)&tracerPayload, sizeof(tracerPayload));
                        }
                        else if (status.containsKey("meter"))
                        {
                            meter meterPayload = parseMeterStruct(status, counterValue);
                            Serial.print("Sending telemetry at t=");
                            Serial.print(meterPayload.t);
                            Serial.println(" ms");
                            uint8_t fPort = 13;
                            scheduleUplink(fPort, (uint8_t *)&meterPayload, sizeof(meterPayload));
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
    }
}

void processDownlink(ostime_t txCompleteTimestamp, uint8_t fPort, uint8_t *data, uint8_t dataLength)
{
    // This function is called from the onEvent() event handler
    // on EV_TXCOMPLETE when a downlink message was received.

    // Implements a 'reset counter' command that can be sent via a downlink message.
    // To send the reset counter command to the node, send a downlink message
    // (e.g. from the TTN Console) with single byte value resetCmd on port cmdPort.

    const uint8_t cmdPort = 100;
    const uint8_t resetCmd = 0xC0;
    const uint8_t getCmd = 0xC1;
    const uint8_t toggleCmd = 0xC2;

    if (fPort == cmdPort && dataLength == 1 && data[0] == resetCmd)
    {
#ifdef USE_SERIAL
        printSpaces(serial, MESSAGE_INDENT);
        serial.println(F("Reset cmd received"));
#endif
        ostime_t timestamp = os_getTime();
        resetCounter();
        printEvent(timestamp, "Counter reset", PrintTarget::All, false);
    }
}

//  █ █ █▀▀ █▀▀ █▀▄   █▀▀ █▀█ █▀▄ █▀▀   █▀▀ █▀█ █▀▄
//  █ █ ▀▀█ █▀▀ █▀▄   █   █ █ █ █ █▀▀   █▀▀ █ █ █ █
//  ▀▀▀ ▀▀▀ ▀▀▀ ▀ ▀   ▀▀▀ ▀▀▀ ▀▀  ▀▀▀   ▀▀▀ ▀ ▀ ▀▀

void setupLmic()
{
    // boardInit(InitType::Hardware) must be called at start of setup() before anything else.
    bool hardwareInitSucceeded = boardInit(InitType::Hardware);

#ifdef USE_DISPLAY
    initDisplay();
#endif

#ifdef USE_SERIAL
    initSerial(MONITOR_SPEED, WAITFOR_SERIAL_S);
#endif

    boardInit(InitType::PostInitSerial);

#if defined(USE_SERIAL) || defined(USE_DISPLAY)
    printHeader();
#endif

    if (!hardwareInitSucceeded)
    {
#ifdef USE_SERIAL
        serial.println(F("Error: hardware init failed."));
        serial.flush();
#endif
#ifdef USE_DISPLAY
        // Following mesage shown only if failure was unrelated to I2C.
        display.setCursor(COL_0, FRMCNTRS_ROW);
        display.print(F("HW init failed"));
#endif
        abort();
    }

    initLmic();

    //  █ █ █▀▀ █▀▀ █▀▄   █▀▀ █▀█ █▀▄ █▀▀   █▀▄ █▀▀ █▀▀ ▀█▀ █▀█
    //  █ █ ▀▀█ █▀▀ █▀▄   █   █ █ █ █ █▀▀   █▀▄ █▀▀ █ █  █  █ █
    //  ▀▀▀ ▀▀▀ ▀▀▀ ▀ ▀   ▀▀▀ ▀▀▀ ▀▀  ▀▀▀   ▀▀  ▀▀▀ ▀▀▀ ▀▀▀ ▀ ▀

    // Place code for initializing sensors etc. here.

#if defined(DEV2)
    WiFi.softAP("TTGO_v2_0002");
#elif defined(DEV3)
    WiFi.softAP("LOPY_0001");
#elif defined(DEV4)
    WiFi.softAP("LOPY_0002");
#else
    WiFi.softAP("MyESP32AP");
#endif

    resetCounter();

    //  █ █ █▀▀ █▀▀ █▀▄   █▀▀ █▀█ █▀▄ █▀▀   █▀▀ █▀█ █▀▄
    //  █ █ ▀▀█ █▀▀ █▀▄   █   █ █ █ █ █▀▀   █▀▀ █ █ █ █
    //  ▀▀▀ ▀▀▀ ▀▀▀ ▀ ▀   ▀▀▀ ▀▀▀ ▀▀  ▀▀▀   ▀▀▀ ▀ ▀ ▀▀

    if (activationMode == ActivationMode::OTAA)
    {
        LMIC_startJoining();
    }

    // Schedule initial doWork job for immediate execution.
    os_setCallback(&doWorkJob, doWorkCallback);
}
